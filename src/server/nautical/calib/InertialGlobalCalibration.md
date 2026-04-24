# Inertial Global Calibration (BNO055-centric)

This document describes a calibration pipeline built around a **BNO055 IMU in AMG mode** (raw accelerometer, gyro, magnetometer), together with GPS, paddle-wheel / STW, and the masthead wind sensor.

Unlike the earlier plan (see `MagnetometerAndHeelingCalib.md`), we **do not rely on a DST810 heel sensor**. The attitude of the boat is recovered jointly with all calibration parameters by fitting a **continuous-time trajectory** to the raw IMU stream, using the accelerometer as a gravity reference, the magnetometer as an absolute-yaw reference, the gyro as a high-rate relative-rotation reference, and the GPS as a world-frame position reference. The same model and solver are used for **calibration** (long window over the whole navigation) and **runtime** (short sliding window).

**Conventions**. Angles in radians unless noted. Speeds in knots. Magnetic field in $\mu T$. World frame is geographic NED.

---

## 1. Sensor strategy and I²C budget

The BNO055 is operated in **AMG mode** so that we get raw, undistorted readings instead of the factory fusion output.

| Stream | Symbol | Used rate | Notes |
|---|---|---|---|
| Accelerometer | $\mathbf{a}_s(t)$ | 100–200 Hz | BW-limited to ~62 Hz internally; higher polling gains little |
| Gyroscope | $\boldsymbol{\omega}_s(t)$ | 100–200 Hz | Same polling loop as accel |
| Magnetometer | $\mathbf{M}_{\text{raw}}(t)$ | ~20 Hz | Hardware ODR cap on BNO055 |
| GPS position + velocity | $\mathbf{p}_{\text{GPS}}$, $\mathbf{V}_{\text{ground}}$ | ~1 Hz | From SOG/COG and LLA |
| Masthead AWA / AWS | $\alpha_{\text{awa}}$, $v_{\text{aws}}$ | ~1 Hz | As in legacy system |
| Paddle-wheel STW | $\text{STW}_{\text{raw}}$ | ~1 Hz | As in legacy system |

At I²C Fast-mode (400 kHz), a burst read of accel+gyro (12 bytes + overhead) takes ~0.4 ms, so 200 Hz is comfortable on a single bus; mag is sampled opportunistically at its native rate. Over a 60-s window, gyro angle random walk integrates to ~0.1°, which is negligible compared to the ±1 °/s raw zero-rate offset — hence the pipeline **always estimates gyro bias as a free parameter**.

---

## 2. Reference frames

Same definitions as `MagnetometerAndHeelingCalib.md`, repeated here for completeness:

1. **Sensor frame $\mathcal{F}_s$** — fixed to the BNO055 die. Axes per the datasheet; physical orientation depends on how the chip is glued to the boat. Raw IMU data lives here.
2. **Boat frame $\mathcal{F}_b$** — $x_b$ forward, $y_b$ to starboard, $z_b$ down through the keel. The rotation $\mathcal{F}_s \to \mathcal{F}_b$ is the constant **alignment** $\mathbf{R}_{\text{align}} \in \mathrm{SO}(3)$: for any vector $\mathbf{v}$ expressed in $\mathcal{F}_s$, its expression in $\mathcal{F}_b$ is $\mathbf{R}_{\text{align}}\,\mathbf{v}$.
3. **Ground frame $\mathcal{F}_g$** — geographic NED at the boat's position; $z_g$ points down (local gravity). GPS ground velocity lives here, as do the WMM constants.
4. **Water frame $\mathcal{F}_w$** — parallel to $\mathcal{F}_g$ but co-moving with the local water mass (pure translation at $\mathbf{V}_{\text{current}}$).

The **trajectory** we estimate is a time-dependent rotation $\mathbf{R}_{gb}(t)\in\mathrm{SO}(3)$ and position $\mathbf{p}_{gb}(t)\in\mathbb{R}^3$ that map the boat frame into the ground frame:

$$
\mathbf{v}\big|_{\mathcal{F}_g} = \mathbf{R}_{gb}(t)\,\mathbf{v}\big|_{\mathcal{F}_b}, \qquad
\text{sensor} \xrightarrow{\mathbf{R}_{\text{align}}} \text{boat} \xrightarrow{\mathbf{R}_{gb}(t)} \text{ground}.
$$

Heel, pitch, and yaw are no longer separate inputs — they are components of $\mathbf{R}_{gb}(t)$.

---

## 3. State

The state has two parts: **time-invariant calibration parameters $\theta$**, and a **time-varying trajectory** plus bias schedule.

### 3.1 Calibration parameters $\theta$ (fixed during a session)

| Symbol | Description | #params | Parameterisation |
|---|---|---|---|
| $\mathbf{b}_{\text{mag}}$ | Magnetometer hard-iron offset | 3 | Free $\mathbb{R}^3$ |
| $W$ | Soft-iron matrix (symmetric $3\times 3$) | 6 | 3 diagonal + 3 off-diagonal |
| $\mathbf{R}_{\text{align}}$ | Sensor-to-boat mounting rotation | 3 | Unit quaternion in Ceres with `QuaternionManifold`; 3 intrinsic DoF |
| $\mathbf{b}_a$ | Accelerometer bias | 3 | Free $\mathbb{R}^3$ (one constant per session) |
| $\mathbf{s}_a$ | Accelerometer diagonal scale | 3 | Optional; prior at $\mathbf{1}$ |
| $k_{\text{stw}}$ | STW heel correction | 1 | Scalar (rad$^{-2}$) |
| $k_{\text{leeway}}$ | Leeway coefficient | 1 | Scalar |
| $\theta_{\text{awa0}}$ | Global AWA mounting offset | 1 | Scalar (rad) |
| $\theta_{\text{twist}}$ | Heel-induced rig twist | 1 | Scalar |
| $\theta_{\text{upwash}}$ | Upwash factor | 1 | Scalar |
| $\theta_{\text{aws0}}$ | AWS additive bias | 1 | Scalar (kn) |
| $\theta_{\text{aws1}}$ | AWS scale deviation | 1 | Scalar |

Total: **25 scalars** (or 22 if accel scale $\mathbf{s}_a$ is held at identity). The aerodynamic block (5 params) and the hydrodynamic block (2 params) are unchanged from the previous plan.

### 3.2 Trajectory representation

Time is discretized at knot instants $t_0 < t_1 < \dots < t_N$ with uniform spacing $\Delta t$ (typical values: $\Delta t = 0.5$ s for calibration; $\Delta t = 0.2$–$0.5$ s for runtime). At every knot the state holds an **absolute** pose:

$$
\mathbf{R}_i \in \mathrm{SO}(3), \qquad \mathbf{p}_i \in \mathbb{R}^3.
$$

Stored in Ceres as unit quaternions with `QuaternionManifold` and 3-vectors respectively. The trajectory is reconstructed at arbitrary query time $t \in [t_i, t_{i+1}]$ by **linear interpolation in each primitive's natural geometry**:

$$
\alpha(t) \;=\; \frac{t - t_i}{\Delta t_i},\qquad \Delta t_i = t_{i+1}-t_i.
$$

**Rotation (SLERP, i.e. constant angular velocity within a segment)**. Define the constant body-frame axis–angle increment

$$
\boldsymbol{\delta}_i \;=\; \operatorname{Log}\!\big(\mathbf{R}_i^{\top}\,\mathbf{R}_{i+1}\big) \;\in\;\mathbb{R}^3,
$$

then

$$
\boxed{\;\mathbf{R}_{gb}(t) \;=\; \mathbf{R}_i\,\operatorname{Exp}\!\big(\alpha(t)\,\boldsymbol{\delta}_i\big).\;}
$$

Differentiating:

$$
\dot{\mathbf{R}}_{gb}(t) \;=\; \mathbf{R}_{gb}(t)\,\big[\boldsymbol{\omega}_b(t)\big]_\times,\qquad
\boldsymbol{\omega}_b(t) \;=\; \frac{\boldsymbol{\delta}_i}{\Delta t_i}
\quad (\text{piecewise-constant within a segment}).
$$

So gyro samples inside a segment see a constant body angular velocity equal to the segment's axis–angle divided by its duration. This is the natural SO(3) analogue of "linear interpolation".

**Position (straight-line segments)**.

$$
\mathbf{p}_{gb}(t) \;=\; (1-\alpha)\,\mathbf{p}_i + \alpha\,\mathbf{p}_{i+1},\qquad
\dot{\mathbf{p}}_{gb}(t) \;=\; \frac{\mathbf{p}_{i+1}-\mathbf{p}_i}{\Delta t_i},\qquad
\ddot{\mathbf{p}}_{gb}(t) \;=\; \mathbf{0}\;\text{(a.e.)}.
$$

The zero acceleration inside a segment means the accelerometer residual (§4.2) models the sensor as reading gravity only; wave-induced accelerations are absorbed into the accel residual's noise budget (and are partially averaged out by the gyro-derotated per-segment aggregation of §4.2). This is adequate for attitude estimation on a sailboat (wave accelerations are $\ll g$ in magnitude and have near-zero mean over a segment). Note that intra-segment rotation itself is *not* treated via SLERP in Block A — §4.2 uses the partial gyro pre-integrations of §4.1 to transport accel samples to a common reference before averaging, so SLERP is only used for position interpolation and for evaluating attitude at non-IMU sample times (Blocks M, P, V, C, T).

### 3.3 Gyro bias schedule (piecewise constant)

Gyro bias drifts primarily with temperature (BNO055 datasheet: ~15 mdps/K), on timescales of minutes. We model it as **piecewise constant over bias-segments** of duration $\Delta t_b$ (typical: 30–60 s). For segment $j$ covering times $[\tau_j, \tau_{j+1})$:

$$
\mathbf{b}_g(t) \;=\; \mathbf{b}_g^{(j)} \in \mathbb{R}^3,\qquad t\in[\tau_j, \tau_{j+1}).
$$

A weak random-walk prior couples consecutive segments (§4.8). No spline is used.

**Grid alignment.** Choose $\Delta t_b$ as an integer multiple of $\Delta t$ (e.g. $\Delta t_b = 60$ s, $\Delta t = 0.5$ s $\Rightarrow$ 120:1) so that bias-segment boundaries coincide with rotation-knot boundaries. Every gyro pre-integration interval (§4.1) then sits entirely inside one bias segment, so a single $\mathbf{b}_g^{(j)}$ enters each residual and we never have to split the integration.

### 3.4 Parameter counts (3-hour calibration run, typical)

With $\Delta t = 0.5$ s and $\Delta t_b = 60$ s:

| Group | Count |
|---|---|
| Time-invariant $\theta$ | 25 |
| Rotation knots $\mathbf{R}_i$ (3 DoF each) | ~21 600 |
| Position knots $\mathbf{p}_i$ | ~21 600 |
| Gyro bias segments $\mathbf{b}_g^{(j)}$ | ~540 |
| **Total scalars** | **~43 800** |

Large but sparse: the per-segment pre-integrated gyro residual (§4.1) and each accel/mag sample residual touch only the two adjacent knots and one bias segment, so the Jacobian is banded and Schur-complement tractable.

---

## 4. Residual blocks

The total cost is

$$
\min_{\theta,\{\mathbf{R}_i\},\{\mathbf{p}_i\},\{\mathbf{b}_g^{(j)}\}}
\sum_{\star} \rho_\star\!\Big(\big\|\mathbf{r}_\star\big\|^2\Big),
$$

with a Cauchy loss $\rho(s)=\ln(1+s)$ on each block for outlier robustness.

### 4.1 Block G — Gyro (pre-integrated)

One 3-vector residual per knot segment $i$, obtained by **on-manifold pre-integration** of the gyro stream between $t_i$ and $t_{i+1}$ (Forster et al., *On-Manifold Preintegration for Real-Time Visual–Inertial Odometry*, T-RO 2017). Under the grid alignment of §3.3, segment $i$ lies entirely inside one bias segment $j$.

**Pre-integration (done once, cached)**. At a linearisation point $\mathbf{b}_g^{(j),*}$ for the bias, run the recursion over the $N_i$ gyro samples in $[t_i, t_{i+1})$:

$$
\Delta\mathbf{R}_s^{(i)} \;=\; \prod_{k=1}^{N_i} \operatorname{Exp}\!\big((\boldsymbol{\omega}_{\text{meas},k} - \mathbf{b}_g^{(j),*})\,\delta t_k\big),
$$

and accumulate the bias Jacobian (Forster eq. (44), gyro-only form):

$$
\mathbf{J}^{(i)}_{b_g} \;=\; -\sum_{k=1}^{N_i}\Big(\!\!\prod_{\ell=k+1}^{N_i}\!\operatorname{Exp}\!\big((\boldsymbol{\omega}_{\text{meas},\ell}-\mathbf{b}_g^{(j),*})\,\delta t_\ell\big)\!\Big)^{\!\!\top}\mathbf{J}_r\!\big((\boldsymbol{\omega}_{\text{meas},k}-\mathbf{b}_g^{(j),*})\,\delta t_k\big)\,\delta t_k,
$$

where $\mathbf{J}_r(\cdot)$ is the right-Jacobian of $\mathrm{SO}(3)$. The pair $(\Delta\mathbf{R}_s^{(i)},\,\mathbf{J}^{(i)}_{b_g})$ is a function of raw measurements and $\mathbf{b}_g^{(j),*}$ only — it does **not** depend on $\mathbf{R}_i,\mathbf{R}_{i+1},\mathbf{R}_{\text{align}}$, and is reused across all Ceres iterations. If during optimisation $\|\mathbf{b}_g^{(j)} - \mathbf{b}_g^{(j),*}\|$ grows beyond a threshold (e.g. a few mdps), relinearise by re-running the recursion.

**First-order bias update** used inside the residual:

$$
\Delta\mathbf{R}_s^{(i)}\!\big(\mathbf{b}_g^{(j)}\big) \;\approx\; \Delta\mathbf{R}_s^{(i)}\,\operatorname{Exp}\!\Big(\mathbf{J}^{(i)}_{b_g}\big(\mathbf{b}_g^{(j)} - \mathbf{b}_g^{(j),*}\big)\Big).
$$

**Residual** (body-frame axis-angle, 3-vector):

$$
\boxed{\;\mathbf{r}_{\text{gyro}}^{(i)} \;=\; \operatorname{Log}\!\Big(\big[\mathbf{R}_{\text{align}}\,\Delta\mathbf{R}_s^{(i)}\!\big(\mathbf{b}_g^{(j)}\big)\,\mathbf{R}_{\text{align}}^{\top}\big]^{\top}\,\mathbf{R}_i^{\top}\,\mathbf{R}_{i+1}\Big).\;}
$$

Equivalently, comparing entirely in sensor frame,

$$
\mathbf{r}_{\text{gyro}}^{(i)} \;=\; \operatorname{Log}\!\Big(\Delta\mathbf{R}_s^{(i)}\!\big(\mathbf{b}_g^{(j)}\big)^{\!\top}\,\mathbf{R}_{\text{align}}^{\top}\,\mathbf{R}_i^{\top}\,\mathbf{R}_{i+1}\,\mathbf{R}_{\text{align}}\Big).
$$

The residual depends on four parameter blocks: $\mathbf{R}_i$, $\mathbf{R}_{i+1}$, $\mathbf{b}_g^{(j)}$, $\mathbf{R}_{\text{align}}$.

**Covariance.** Propagated by the same recursion:

$$
\boldsymbol{\Sigma}_g^{(i)} \;=\; \sum_{k=1}^{N_i}\!\Big(\!\!\prod_{\ell=k+1}^{N_i}\!\mathbf{A}_\ell\!\Big)^{\!\!\top}\!\mathbf{J}_r\!\big((\boldsymbol{\omega}_{\text{meas},k}-\mathbf{b}_g^{(j),*})\,\delta t_k\big)\,\sigma_g^2\,\delta t_k^{\,2}\,\mathbf{J}_r\!(\cdot)^{\top}\Big(\!\!\prod_{\ell=k+1}^{N_i}\!\mathbf{A}_\ell\!\Big),
$$

$$
\mathbf{A}_k \;=\; \operatorname{Exp}\!\big((\boldsymbol{\omega}_{\text{meas},k} - \mathbf{b}_g^{(j),*})\,\delta t_k\big)^{\!\top}.
$$

To first order, for small per-sample rotations, $\boldsymbol{\Sigma}_g^{(i)} \approx \sigma_g^2\,\bar{\delta t}\,\Delta t_i\,\mathbf{I}$ with $\bar{\delta t}$ the mean inter-sample interval. With $\sigma_g \approx 0.005$ rad/s (BNO055 ARW at 100 Hz), $\bar{\delta t}=0.01$ s, $\Delta t_i = 0.5$ s, this gives $\sigma_{\text{seg}}\approx 3.5\times10^{-4}$ rad. The residual is whitened by $\boldsymbol{\Sigma}_g^{(i),-1/2}$ before the Cauchy loss.

**Rationale.** The SLERP interpolation in §3.2 can only represent piecewise-constant body-frame angular velocity within a segment; the integrated net rotation is the maximal information the trajectory model can absorb from the gyro, and that is exactly what the residual above constrains. Per-sample residuals would additionally force the high-frequency content of $\boldsymbol{\omega}_{\text{meas}}$ against a piecewise-constant ansatz, producing correlated residuals that bias $\boldsymbol{\delta}_i$ without improving attitude. Pre-integration also reduces the gyro residual count from $\Delta t_i / \delta t \sim 50\text{–}100$ per segment to one, shrinking the Ceres problem by roughly two orders of magnitude on the gyro block while retaining the full information content usable by the model.

### 4.2 Block A — Accelerometer (per-segment, gyro-derotated)

One 3-vector residual per knot segment $i$. Rather than averaging raw accel samples (which would tacitly assume no rotation within the segment), we use the **partial gyro pre-integrations** $\Delta\mathbf{R}_s^{(i,k)}$ already computed in §4.1 to transport every sample back to the sensor frame at $t_i$ before averaging. Under the gravity-only model ($\ddot{\mathbf{p}}_{gb}=\mathbf{0}$ a.e.), all de-rotated samples predict the same vector — gravity at $\mathbf{R}_i$ — so averaging is information-preserving.

**Partial pre-integrations (cached alongside §4.1).** During the gyro recursion, also record the running product at every gyro-sample index $k\in\{0,\dots,N_i\}$:

$$
\Delta\mathbf{R}_s^{(i,k)} \;=\; \prod_{\ell=1}^{k}\operatorname{Exp}\!\big((\boldsymbol{\omega}_{\text{meas},\ell}-\mathbf{b}_g^{(j),*})\,\delta t_\ell\big)\;\in\;\mathrm{SO}(3),\qquad \Delta\mathbf{R}_s^{(i,0)} = \mathbf{I},
$$

with $\Delta\mathbf{R}_s^{(i,N_i)} = \Delta\mathbf{R}_s^{(i)}$ from §4.1. Accel samples are assumed co-timestamped with gyro samples (same polling loop, §1); if that is not the case, interpolate $\Delta\mathbf{R}_s^{(i,\cdot)}$ at accel timestamps.

**Per-segment statistics (cached, function of raw measurements and $\mathbf{b}_g^{(j),*}$ only).**

$$
\mathbf{M}^{(i)} \;:=\; \frac{1}{N_i}\sum_{k=1}^{N_i}\Delta\mathbf{R}_s^{(i,k)}\,\operatorname{diag}\!\big(\mathbf{a}_{\text{meas},k}\big)\;\in\;\mathbb{R}^{3\times 3},\qquad
\mathbf{C}^{(i)} \;:=\; \frac{1}{N_i}\sum_{k=1}^{N_i}\Delta\mathbf{R}_s^{(i,k)}\;\in\;\mathbb{R}^{3\times 3}.
$$

Derivation: the de-rotated, bias/scale-corrected specific force at sample $k$ is $\Delta\mathbf{R}_s^{(i,k)}(\operatorname{diag}(\mathbf{s}_a)\,\mathbf{a}_{\text{meas},k} - \mathbf{b}_a) = \Delta\mathbf{R}_s^{(i,k)}\,\operatorname{diag}(\mathbf{a}_{\text{meas},k})\,\mathbf{s}_a - \Delta\mathbf{R}_s^{(i,k)}\,\mathbf{b}_a$; averaging over $k$ gives $\mathbf{M}^{(i)}\mathbf{s}_a - \mathbf{C}^{(i)}\mathbf{b}_a$, and all samples share the same predicted value $-\mathbf{R}_{\text{align}}^{\top}\mathbf{R}_i^{\top}\mathbf{g}_w$.

**Residual** (3-vector, in the sensor frame at $t_i$):

$$
\boxed{\;\mathbf{r}_{\text{acc}}^{(i)} \;=\; \mathbf{M}^{(i)}\,\mathbf{s}_a \;-\; \mathbf{C}^{(i)}\,\mathbf{b}_a \;-\; \mathbf{R}_{\text{align}}^{\top}\,\mathbf{R}_i^{\top}\,(-\mathbf{g}_w).\;}
$$

Depends on three parameter blocks: $\mathbf{R}_i$, $\mathbf{R}_{\text{align}}$, $\mathbf{b}_a$ (and $\mathbf{s}_a$ when enabled). Note that **$\mathbf{R}_{i+1}$ does not appear** — the partial gyro integrations have subsumed what SLERP would have contributed between $\mathbf{R}_i$ and $\mathbf{R}_{i+1}$. By symmetry one could anchor at the segment end instead, using $\Delta\mathbf{R}_s^{(i,k),\top}\,\Delta\mathbf{R}_s^{(i)}$ and $\mathbf{R}_{i+1}$; the two choices are algebraically equivalent.

**Covariance.** Orthogonal de-rotation preserves isotropic covariance, so

$$
\boldsymbol{\Sigma}_a^{(i)} \;\approx\; \Big(\tfrac{\sigma_{a,\text{noise}}^2}{N_i}\;+\;\sigma_{a,\text{wave}}^2\Big)\,\mathbf{I},
$$

where $\sigma_{a,\text{noise}}\approx 0.01\,\text{m/s}^2$ (BNO055 accel noise, averages down as $1/\sqrt{N_i}$) and $\sigma_{a,\text{wave}}\in[0.3,1]\,\text{m/s}^2$ (wave process noise, only partially averaged on sub-swell timescales). For calibration keep $\sigma_a\in[0.3,1]\,\text{m/s}^2$; Cauchy loss on top clips isolated shocks.

**Gyro-bias coupling.** The partial rotations $\Delta\mathbf{R}_s^{(i,k)}$ were pre-integrated at $\mathbf{b}_g^{(j),*}$, so $\mathbf{M}^{(i)}$ and $\mathbf{C}^{(i)}$ implicitly depend on that linearisation point. The sensitivity is tiny: a 5 mdps bias error integrated over 0.25 s gives a partial-rotation error of ~$2\times10^{-5}$ rad, rotating gravity by ~$2\times10^{-4}\,\text{m/s}^2 \ll \sigma_a$. We therefore treat $\mathbf{M}^{(i)},\mathbf{C}^{(i)}$ as constants between relinearisations of $\mathbf{b}_g^{(j),*}$, relinearising at the same cadence as §4.1.

**Information content.** Aggregating $N_i$ per-sample residuals into one de-rotated mean is **lossless for attitude and for $\mathbf{b}_a$** under the gravity-only model (all per-sample residuals predict the same sensor-frame vector after de-rotation; their sum is a sufficient statistic for $\mathbf{R}_i,\mathbf{R}_{\text{align}},\mathbf{b}_a$). The only quantity that *could* benefit from the intra-segment scatter is the accel diagonal scale $\mathbf{s}_a$, which draws a weak signal from how the accel axes project differently onto the gravity direction as the boat rotates within a segment. That signal is retained in full by across-segment variation (segments on different attitudes still see $\mathbf{R}_i^{\top}\mathbf{g}_w$ in different sensor directions), so §7's observability story for $\mathbf{s}_a$ carries through unchanged.

**Rationale.** This removes the mild "pretend-no-rotation" bias of a naive segment mean, and the SLERP-based $\Phi(\boldsymbol{\delta}_i)$ approximation of an earlier draft — SLERP assumes a constant body-frame rate across the segment, whereas the real $\boldsymbol{\omega}_b(t)$ varies (especially in chop). The gyro already measures the true rotation history, so using its partial pre-integrations to transport accel samples to a common frame is the natural and more accurate thing to do, at no extra per-iteration cost (the $\Delta\mathbf{R}_s^{(i,k)}$ are cached once).

### 4.3 Block M — Magnetometer (3-vector)

Let the World Magnetic Model deliver the full field vector in NED:

$$
\mathbf{B}_w \;=\; B_{\text{earth}}\,\begin{pmatrix}\cos I\,\cos D\\ \cos I\,\sin D\\ \sin I\end{pmatrix},
$$

where $I = I_{\text{expected}}$ is inclination (positive down) and $D = D_{\text{mag}}$ is declination (east-positive). Then for each mag sample:

$$
\boxed{\;\mathbf{r}_{\text{mag}}(t) \;=\; W\big(\mathbf{M}_{\text{raw}}(t) - \mathbf{b}_{\text{mag}}\big) \;-\; \mathbf{R}_{\text{align}}^{\top}\,\mathbf{R}_{gb}(t)^{\top}\,\mathbf{B}_w.\;}
$$

3-vector residual. This single residual replaces the old **sphere-fit + dip-angle** pair: the norm, the dip angle, **and** the horizontal direction of $\mathbf{B}_w$ are all constrained simultaneously. Weight: isotropic with $\sigma_M$ ≈ 1 $\mu T$ (BNO055 mag noise plus local environmental jitter).

### 4.4 Block P — GPS position

At each GPS fix $t_k$:

$$
\mathbf{r}_{\text{gps}}(t_k) \;=\; \mathbf{p}_{gb}(t_k) \;-\; \mathbf{p}_{\text{GPS}}(t_k).
$$

3-vector, after projecting GPS LLA to a local NED origin. Weight reflects GPS horizontal and vertical variance.

### 4.5 Block V — GPS velocity

Using $\dot{\mathbf{p}}_{gb}$ from §3.2:

$$
\mathbf{r}_{\text{vel}}(t_k) \;=\; \dot{\mathbf{p}}_{gb}(t_k) \;-\; \mathbf{V}_{\text{ground}}(t_k).
$$

3-vector. Velocity errors (from SOG/COG) are typically 0.05–0.1 kn on modern receivers.

### 4.6 Block C — Water-current consistency

The hydrodynamic residual is unchanged in spirit from `MagnetometerAndHeelingCalib.md` §2.2 and Block 3, but the heel angle is no longer an external input. Extract heel from $\mathbf{R}_{gb}(t)$ via the NED-to-body roll component, and heading as the azimuth of the boat's forward axis:

$$
\mathbf{x}_g(t) \;=\; \mathbf{R}_{gb}(t)\,\mathbf{e}_x,\qquad
\psi_{\text{true}}(t) \;=\; \operatorname{atan2}\!\big([\mathbf{x}_g]_E,\,[\mathbf{x}_g]_N\big).
$$

For heel, take the rotation that brings the body's $z$-axis to alignment with local gravity and read its roll component (pitch assumed zero and absorbed into the trajectory). Then:

$$
\text{STW}_{\text{corr}}(t) \;=\; \text{STW}_{\text{raw}}(t)\,\big(1 + k_{\text{stw}}\,\phi_{\text{heel}}(t)^2\big),
$$
$$
\gamma(t) \;=\; k_{\text{leeway}}\,\frac{\phi_{\text{heel}}(t)}{\text{STW}_{\text{corr}}(t)^2 + \varepsilon},\qquad
\alpha_{\text{ctw}}(t) \;=\; \psi_{\text{true}}(t) + \gamma(t),
$$
$$
\mathbf{V}_{\text{boat\_water}}(t) \;=\; \operatorname{polar}\!\big(\text{STW}_{\text{corr}}(t),\,\alpha_{\text{ctw}}(t)\big),
$$
$$
\mathbf{V}_{\text{current}}(t) \;=\; \mathbf{V}_{\text{ground}}(t) - \mathbf{V}_{\text{boat\_water}}(t),
$$
$$
\boxed{\;\mathbf{r}_{\text{current}}(t) \;=\; \mathbf{V}_{\text{current}}(t) - \mathbf{V}_{\text{current}}(t - \Delta t_c).\;}
$$

Evaluated at 1 Hz with $\Delta t_c \approx 10$ s. The residual is a 2-vector (N, E).

### 4.7 Block T — True-wind tack consistency

Identical to `MagnetometerAndHeelingCalib.md` §Block 4, with the apparent-wind pipeline of §2.3 and the true-wind composition of §2.4 applied unchanged. Heading $\psi_{\text{true}}$ now comes from $\mathbf{R}_{gb}$ as above.

$$
\mathbf{r}_{\text{tack}}^{(i)} \;=\; w_i\big(\mathbf{V}_{W,\text{after}}^{(i)} - \mathbf{V}_{W,\text{before}}^{(i)}\big).
$$

### 4.8 Block B — Bias smoothness prior

Between consecutive gyro-bias segments:

$$
\mathbf{r}_{\text{bias}}^{(j)} \;=\; \frac{\mathbf{b}_g^{(j+1)} - \mathbf{b}_g^{(j)}}{\sqrt{\Delta t_b}},
$$

so that the implied process noise is $\sigma_{\dot b} = 1$ (dimensionless) and the absolute scale is set via the residual weight. Accelerometer bias is held constant over a session (or an equivalent prior is added if it varies).

---

## 5. Heading, heel and roll at output time

Once the solver has delivered $\theta$ and the knots, all legacy outputs are simple projections of $\mathbf{R}_{gb}(t)$:

**True heading** (preferred form — byproduct of the fit):

$$
\psi_{\text{true}}(t) \;=\; \operatorname{atan2}\!\big([\mathbf{R}_{gb}(t)\,\mathbf{e}_x]_E,\,[\mathbf{R}_{gb}(t)\,\mathbf{e}_x]_N\big).
$$

**Heel angle** (rotation about boat-$x$ needed to bring boat-$z$ into the vertical plane containing boat-$x$):

$$
\phi_{\text{heel}}(t) \;=\; \operatorname{atan2}\!\big([\mathbf{R}_{gb}(t)^{\top}\hat{\mathbf{z}}_g]_y,\,[\mathbf{R}_{gb}(t)^{\top}\hat{\mathbf{z}}_g]_z\big).
$$

### 5.1 Solver-free shortcut (fallback)

If you want a single-sample heading without running the window solver — useful for self-tests or as a fallback on hardware that cannot afford Ceres — use the user's original formulation directly:

$$
\hat{\mathbf{z}}_s \;=\; \frac{\mathbf{a}_{\text{meas}}}{\|\mathbf{a}_{\text{meas}}\|},\quad
\mathbf{x}_{b,s} \;=\; \mathbf{R}_{\text{align}}^{\top}\,\mathbf{e}_x,\quad
\mathbf{M}_c \;=\; W(\mathbf{M}_{\text{raw}} - \mathbf{b}_{\text{mag}}).
$$
$$
\mathbf{x}_\perp \;=\; \mathbf{x}_{b,s} - (\mathbf{x}_{b,s}\cdot\hat{\mathbf{z}}_s)\,\hat{\mathbf{z}}_s,\qquad
\mathbf{M}_\perp \;=\; \mathbf{M}_c - (\mathbf{M}_c\cdot\hat{\mathbf{z}}_s)\,\hat{\mathbf{z}}_s.
$$
$$
\psi_{\text{mag}} \;=\; \operatorname{atan2}\!\big((\mathbf{x}_\perp\times\mathbf{M}_\perp)\cdot\hat{\mathbf{z}}_s,\,\mathbf{x}_\perp\cdot\mathbf{M}_\perp\big),\qquad
\psi_{\text{true}} \;=\; \psi_{\text{mag}} + D_{\text{mag}}.
$$

This shortcut treats the accelerometer as a pure gravity sensor, so it is degraded during wave shocks; the full window solver does not have this limitation.

---

## 6. Runtime strategy

Calibration and runtime share the exact same residuals, differing only in **horizon** and **knot density**:

| Use case | Window | $\Delta t$ (rotation / position knots) | $\Delta t_b$ |
|---|---|---|---|
| Offline calibration | Whole session (hours) | 0.5 s | 60 s |
| Runtime (nominal) | 60–180 s sliding | 0.2–0.5 s | 60 s |
| Runtime (tight CPU budget) | — | switch to ESKF with identical state + measurement models | — |

**Sliding-window update loop** (runtime):

1. At each new GPS epoch (~1 Hz), slide the window forward by one or more knots.
2. Initialise new knots by extrapolating the last two: $\mathbf{p}_{N+1}=\mathbf{p}_N + (\mathbf{p}_N-\mathbf{p}_{N-1})$, $\mathbf{R}_{N+1}=\mathbf{R}_N\,\operatorname{Exp}(\boldsymbol{\delta}_{N-1})$.
3. Warm-start Ceres from the previous solution; run a small fixed number of iterations (2–5) — we are near the optimum after the first window.
4. Drop the oldest knot (or keep a first-order "marginalisation prior" on it for MAP consistency).

Calibration parameters $\theta$ are shared across runtime windows; they are re-optimised during calibration passes and held fixed (or only slowly updated) at runtime.

---

## 7. Observability and gauge notes

- **Soft-iron rotational ambiguity ($W$)**. Block M (3-vector) now fixes both $W^{\top}W$ and the orientation of $W$ via the *horizontal* component of $\mathbf{B}_w$. The rotational ambiguity that existed when only $\|W(\mathbf{M}-\mathbf{b})\|$ was constrained is gone — $W$ can be parameterised as general $3\times3$ without degeneracy, provided the session contains heading variation. We still prefer the symmetric parameterisation (6 DoF) as a prior: a non-symmetric $W$ has little physical justification for soft iron, and would be absorbed into $\mathbf{R}_{\text{align}}$ anyway.
- **Yaw of $\mathbf{R}_{\text{align}}$**. Observed by the horizontal component of Block M (via $D_{\text{mag}}$); no longer relies on tacks to be identified. Block T (tack consistency) is kept purely to identify aerodynamic parameters.
- **Pitch/roll of $\mathbf{R}_{\text{align}}$**. Observed by Block A (gravity direction in sensor frame) and Block M (dip angle). A stationary boat already constrains all three alignment DoF once motion excites the magnetometer azimuth.
- **Gyro bias $\mathbf{b}_g$ vs. orientation rate**. Jointly observable whenever either (a) Block A anchors absolute attitude within the bias segment, or (b) Block M does. With our segment length $\Delta t_b = 60$ s, the accel/mag anchors are always present; bias is well-identified.
- **Accel bias $\mathbf{b}_a$ vs. gravity magnitude**. Not fully separable at rest: $\|\mathbf{a}-\mathbf{b}_a\|=g$ admits a 2D surface of solutions. Motion (GPS-observed acceleration, or rotation through varied attitudes) is needed. Practical recipe: apply a tight prior $\|\mathbf{b}_a\|<0.2$ m/s² and let Block P+V supply excitation.
- **Leeway vs. current**. Same subtlety as in the previous plan: a constant-in-time error in $k_{\text{leeway}}$ is indistinguishable from a slowly time-varying current when heading is constant over the current-consistency window. Mitigated by requiring heading diversity per window.
- **STW heel correction ($k_{\text{stw}}$)**. Requires periods of significant heel; motoring flat provides no constraint.
- **Accel scale $\mathbf{s}_a$**. Only observable if (a) the boat experiences different orientations exposing each axis to gravity in turn (tacking with strong heel, some pitch excursions), or (b) explicit manoeuvres (e.g. a pre-session "calibration dance"). Otherwise hold at $\mathbf{1}$ with a tight prior.

---

## 8. What changes vs. `MagnetometerAndHeelingCalib.md`

| Aspect | Old plan | This plan |
|---|---|---|
| Heel/pitch source | DST810 (per-sample input) | Byproduct of $\mathbf{R}_{gb}(t)$ |
| IMU mode | Mag only; gyro+accel unused | Full AMG; gyro+accel drive attitude |
| Magnetic residual | 2 scalar residuals (norm + dip) | 1 vector residual (3 DoF) |
| Rotation representation | Not represented; implicit in $\psi_{\text{true}}$ | Explicit SO(3) spline on $\{\mathbf{R}_i\}$ |
| Alignment DoF | 3 (exp map) | 3 (unit quaternion + `QuaternionManifold`) |
| Gyro bias | Not modelled | 3 DoF per 60-s segment |
| Gyro residual | — (not used) | Pre-integrated segment residual (Forster-style) |
| Accel residual | — (not used) | Per-segment, gyro-derotated sample mean vs. gravity at $\mathbf{R}_i$ |
| Accel bias | Not modelled | 3 DoF per session |
| Runtime | Not specified | Sliding window of same solver |
| Required sensors | BNO055 + DST810 + GPS + masthead | BNO055 + GPS + masthead + paddle-wheel (DST810 dropped) |

---

## 9. Open points

- **Accel bias time variation**. A single constant per session may be too rigid over several hours with temperature swings. If residual analysis shows drift, upgrade $\mathbf{b}_a$ to the same piecewise-constant schedule as $\mathbf{b}_g$.
- **Knot density tuning**. $\Delta t = 0.5$ s is a starting value; wave-dominated conditions may require 0.2 s. Drive this choice from residual whitening: if gyro residuals show structure at sub-knot frequency, refine knots.
- **Outlier handling on GPS position**. Cauchy loss already in place; consider a robust estimator on Block V for receivers that report velocity only when differential fix is available.
- **Time synchronisation**. Magnetometer samples arrive at the slowest rate; ensure their timestamps reference the same clock as the IMU polling loop to within ≤ 5 ms, otherwise Block M will pull $\mathbf{R}_{gb}$ toward stale attitudes during fast rotations.

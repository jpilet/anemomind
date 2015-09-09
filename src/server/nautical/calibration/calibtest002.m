add_calib_paths;

dataset = 0;
raw = true;

[Aw, Bw] = get_calib_ds(2*dataset + 0, raw);
[Ac, Bc] = get_calib_ds(2*dataset + 1, raw);

assert(size(Aw, 1) == size(Ac, 1));

%%
params = optimize_trajectory_smoothness_wind_and_current(Aw, Bw, Ac, Bc)

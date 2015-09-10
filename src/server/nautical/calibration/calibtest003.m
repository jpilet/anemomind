add_calib_paths;

ds = 0;

raw = true;
[Aw, Bw] = get_calib_ds(2*ds + 0, raw);
[Ac, Bc] = get_calib_ds(2*ds + 1, raw);
n = get_observation_count(Aw);

%%
Awc = [Aw -Ac];
params = [1 0 0 0 1 0 0 0]';

subplot(1, 3, 1);
Awc_cum = cumulative_row_sum(Awc, 2);
data = get_array(Awc_cum*params, 2);
plotx(data);
hold on
plotx(data, '.r');
hold off
axis equal;
subplot(1, 3, 2);
Aw_cum = cumulative_row_sum(Aw, 2);
plotx(get_array(Aw_cum*[1 0 0 0]', 2));
axis equal;
subplot(1, 3, 3);
Ac_cum = cumulative_row_sum(Ac, 2);
plotx(get_array(Ac_cum*[1 0 0 0]', 2));
axis equal;
figure;
plotx(get_array(cumulative_row_sum(Bw, 2), 2));

%%
lambda = 160000;
basis = gram_schmidt(Awc);
basis_cum = cumulative_row_sum(basis, 2);
S = make_smooth(basis_cum, 2, 2, lambda);
Q = S - basis_cum;

params0 = calc_smallest_eigvec(Q'*Q);

%%

[params, flows, filtered_flows] = calibrate_locally(A, B, 3);
plotx(get_array(cumulative_row_sum(flows, 2), 2), 'r');
hold on
plotx(get_array(cumulative_row_sum(filtered_flows, 2), 2), 'b');
%plotx(get_array(filtered_flows, 2), 'b');
hold off


%%
flows = sliding_window_calibration(A, B, 3, 1000);

add_calib_paths;
format long g
raw = true;
[A, B] = get_calib_ds(1, raw);
n = get_observation_count(A);
%%
window_size = 1000;
r = make_ranges(n, window_size, 0.1);

mean_inds = floor(mean(r, 2));


settings = make_cross_validated_settings();


%true_flows = compute_local_flows(A, B, r, settings);

%sorted_flow_norms = sort(calc_row_norms(true_flows));
%plot(sorted_flow_norms(1:floor(numel(sorted_flow_norms)/2)));

%% Optimize it
%params = optimize_cross_validated(A, B, r, settings);
params = optimize_cross_validated2(A, B, r, settings);

Ac = cumulative_row_sum(A, 2);
Bc = cumulative_row_sum(B, 2);
plotx(get_array(Ac*params + Bc, 2));
hold on
plotx(get_array(Ac*[1 0 0 0]' + Bc, 2), 'k');
hold off

%% Optimize cross validated 2


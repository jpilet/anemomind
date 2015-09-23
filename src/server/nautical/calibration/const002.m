% 0: Wind1
% 1: Current1
% 2: Wind2
% 3: Current2
% 4: Wind(synth)
% 5: Current(synth)
add_calib_paths;
format long g
raw = true;
[A, B] = get_calib_ds(0, raw);
n = get_observation_count(A);
%%
window_size = 100;
r = make_ranges(n, window_size, 0.1);

mean_inds = floor(mean(r, 2));

%%

settings = make_constancy_settings();
settings.common_scale = true;
settings.visualize = 2.0;
settings.cumulative = false;
E = calc_flow_constancy_errors(A, B, r, settings);

%%
[sorted_errors, orders] = visualize_flow_constancy_errors(E);

best_inds = mean_inds(orders(1:200));
traj = get_array(cumulative_row_sum(B, 2), 2);

plotx(traj);
hold on
plotx(traj(best_inds, :), '.r', 'MarkerSize', 12);
hold off
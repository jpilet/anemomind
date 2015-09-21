% 0: Wind1
% 1: Current1
% 2: Wind2
% 3: Current2
% 4: Wind(synth)
% 5: Current(synth)
add_calib_paths;

raw = true;
[A, B] = get_calib_ds(0, raw);
n = get_observation_count(A);
%%
window_size = 1000;
r = make_ranges(n, window_size, 0.1);

E = calc_flow_constancy_errors(A, B, r);

%%
visualize_flow_constancy_errors(E);
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

%%

settings = make_locally_constant_settings();

r_temp = [1 10; 11 20; 21 30];

calibrate_locally_constant(A, B, r_temp, settings);
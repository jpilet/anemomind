add_calib_paths;

raw = true;
[A, B] = get_calib_ds(0, raw);
A2 = A(:, 1:2);
%%

n = get_observation_count(A);
ranges = make_random_ranges(10000, n, 1000);

%%

close all;
angle = calibrate_angle(A2, B, ranges);
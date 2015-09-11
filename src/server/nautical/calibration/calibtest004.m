add_calib_paths;

ds = 2;

raw = true;
[Aw, Bw] = get_calib_ds(2*ds + 0, raw);
[Ac, Bc] = get_calib_ds(2*ds + 1, raw);
n = get_observation_count(Aw);
%%
close all;
lambda = 6000;
[wind_params, current_params] = solve_joint(Aw, Bw, Ac, Bc, lambda)
add_calib_paths;

raw = true;
[A, B] = get_calib_ds(1, raw);
n = get_observation_count(A);
%%
settings = make_indeparam_settings();

[indeparams, R] = make_indeparams(A, B, settings);


ground_truth_parameters = [1 0 0 0]';

X = [1 0 0 0]';
Y = R*X;
plot_indeparams(indeparams, Y);
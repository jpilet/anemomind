add_calib_paths;

raw = true;
[A, B] = get_calib_ds(5, raw);
n = get_observation_count(A);
%%
settings = make_refit_settings();
settings.min_length = 10;
settings.max_length = 100;
settings.count = 10000;
refits = make_parameter_refits(A(:, 1:4), B, settings);


%%
plot_refits(refits, [], [3 4]);
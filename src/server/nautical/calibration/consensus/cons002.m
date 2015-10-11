add_calib_paths;

raw = true;
[A, B] = get_calib_ds(1, raw);
n = get_observation_count(A);
%%
settings = make_indeparam_settings();
settings.max_length = 100;
settings.count = 10000;


[indeparams, R] = make_indeparams(A, B, settings);


ground_truth_parameters = [1 0 1 0]';

X = [1 0 0 0]';


if false,
    Y = R*X;
    plot_indeparams(indeparams, Y);
end

plot_indeparam_scales(indeparams, X);


%%
figure;
plot_flow(A, B, X);
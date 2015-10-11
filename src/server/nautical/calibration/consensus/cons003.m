add_calib_paths;

raw = true;
[A, B] = get_calib_ds(0, raw);
n = get_observation_count(A);
%%
settings = make_indeparam_settings();

Q = gram_schmidt(A);
R = Q'*A;
indeparam_optimizers = make_indeparam_optimizers(A, B, settings);



%%
close all;
X = [1 0 -50 0]';
plot_indeparam_optimizers(indeparam_optimizers, X);
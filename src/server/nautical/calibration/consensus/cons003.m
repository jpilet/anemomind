add_calib_paths;

raw = true;
[A, B] = get_calib_ds(2, raw);
n = get_observation_count(A);
%%
settings = make_indeparam_settings();

AB = [A B];
Q = gram_schmidt(AB);
R = Q'*AB;
q = zeros(size(B));
indeparam_optimizers = make_indeparam_optimizers(A, B, settings);



%%
close all;

X = [1 0 -50 -100]';

plot_indeparam_optimizers(indeparam_optimizers, X);

Xtrue = R\X;
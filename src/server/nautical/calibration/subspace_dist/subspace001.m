add_calib_paths;

raw = true;
[A, B] = get_calib_ds(3, raw);
n = get_observation_count(A);

%%
Q = A(:, 1);

settings = make_flow_subspace_settings();
settings.count = 10000;
settings.min_length = 100;
settings.max_length = 10;
[Yest, angles] = minimize_flow_subspace_dists(Q, B, settings);

good = -4 < Yest & Yest < 4;
hist(Yest(good), 90);

%%
plotx([Yest(good) cos(angles(good))], '.k');
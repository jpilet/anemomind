% 0: Wind1
% 1: Current1
% 2: Wind2
% 3: Current2
% 4: Wind(synth)
% 5: Current(synth)

params_4 = [0.80673 -0.216292 -0.0438555 0.0241282]';
params_2 = [0.0671674 0.0139468 2.44059 -0.854015]';

add_calib_paths;

raw = false;
[A, B] = get_calib_ds(2, raw);

n = get_observation_count(A);
X = boolean(kron(ones(n, 1), [1; 0]));
Y = boolean(kron(ones(n, 1), [0; 1]));
%%

%r = make_range(1, 8000);
r = make_range(1, n);
%r = make_range(400, 600);
Ar = A(r, :);
Br = B(r, :);

%params = optimize_mean(A, B)
params = [1, 0, 0, 0];
%params = [0, 0, 0, 0];
%params = params_2
%params = params_4;
%params = minimize_total_length(A, B)
%params = minmize_compactness(A, B)
%params = decorr_calib_B(A, B, 500)

%%
trajectory = integrate_trajectory(get_array(A*params(:) + B, 2));
plotx(trajectory);

%%
disp_for_parameters(Ar, Br, params);

params
params_4

%%
format long g
sum = calc_angle_sum(A*params(:) + B)
%%
X = reshape(A*params(:) + B, 2, n)';
dX = make_difs_sparse(n, 1)*X;
Y = calc_row_norms(dX);
hist(Y, 300);

%%
dim = 1;
for i = 1:4,
    data = reshape(Ar(:, i), 2, numel(r)/2)';
    subplot(5, 1, i);
    plot(data(:, dim))
end
subplot(5, 1, 5);
datab = reshape(Br(:, 1), 2, numel(r)/2)';
plot(datab(:, dim));

%% Hyfsat på syntetiska data
difs = kron(make_difs_sparse(n, 1), eye(2));
params = -(difs*A)\(difs*B)
%% Ej bra på syntetiska data
K = difs*[A B];
params = calc_smallest_eigvec(K'*K);
params = (1.0/params(end))*params


%%
V = get_array(A(:, 3), 2);
plot(V(:, 1), V(:, 2));
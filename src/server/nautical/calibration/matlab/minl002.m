add_calib_paths;
%% Compute the total length of vector AX

X = [1 0 0 0]';

A = randn(10, 4);

f = @(X)(calc_total_length_with_J(A, X, 2));

[F, J] = f(X);

Jnum = jacobian_numeric(f, X);

fprintf('Jacobian dif\n');
disp(J - Jnum);

%% Normalize a vector by its total length

A = randn(10, 4);

X = [1 0 0 0]';
f = @(X)(normalize_by_total_length(A, X, 2));

[F, J] = f(X);
Jnum = jacobian_numeric(f, X);


norms = calc_row_norms(get_array(F, 2));
fprintf('Total length: %.3g\n', sum(norms));

fprintf('Jacobian dif\n');
disp(J - Jnum);

%% The full objective function.
A = randn(10, 4);
B = randn(10, 1);
[objf, X] = make_minlength_objf(A, B);

[F, J] = objf(X);
Jnum = jacobian_numeric(objf, X);

%%
raw = true;
[A, B] = get_calib_ds(0, raw);


[objf, Xinit, drawfun] = make_minlength_objf(A, B);
settings = make_default_levmar_settings();
settings.drawfun = drawfun;
Xh = run_levmar(objf, Xinit, settings);
X = (1/Xh(end))*Xh(1:(end-1));

Ac = cumulative_row_sum(A, 2);
Bc = cumulative_row_sum(B, 2);
plotx(get_array(Ac*X + Bc, 2));

%%
X = randn(10, 1);
v = normalize_vectors(X, 2);
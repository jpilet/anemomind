function X = minimize_dist2d_subj_to_homog(A)
    dim = 2;
    iters = 30;
    [m, n] = size(A);
    X = normalize_vector(ones(n, 1));
    for i = 1:iters,
        R2 = reshape(A*X, dim, m/dim)';
        residuals = calc_row_norms(R2);
        weights = kron(calc_weights(residuals), [1 1]');
        Q = weights(:, ones(1, n)).*A;
        X = calc_smallest_eigvec(Q'*Q);
    end
end

function w = calc_weights(x)
    mu = 1.0e-5;
    x = abs(x);
    x(x < mu) = mu;
    w = sqrt(1.0/2.0*x);
end
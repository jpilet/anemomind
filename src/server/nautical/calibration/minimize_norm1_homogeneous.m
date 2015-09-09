function X = minimize_norm1_homogeneous(A, dim, iters)
    if nargin < 4,
        iters = 30;
    end
    [arows, acols] = size(A);
    X = calc_smallest_eigvec(A'*A);
    for i = 1:iters,
        residuals = A*X;
        w = calc_norm1_weights(residuals, dim);
        sA = scale_rows(w, A);
        X = calc_smallest_eigvec(sA'*sA);
    end
end
function [X, AtA] = minimize_norm1_homogeneous(A, dim, iters)
    if nargin < 4,
        iters = 30;
    end
    [arows, acols] = size(A);
    X = calc_smallest_eigvec(A'*A);
    for i = 1:iters,
        fprintf('norm1 min iter %d/%d\n', i, iters);
        residuals = A*X;
        w = calc_norm1_weights(residuals, dim);
        sA = scale_rows(w, A);
        AtA = sA'*sA;
        X = calc_smallest_eigvec(AtA);
    end
end
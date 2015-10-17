function X = minimize_norm1(A, B, dim, iters)
    if nargin < 4,
        iters = 30;
    end
    [arows, acols] = size(A);
    [brows, bcols] = size(B);
    assert(arows == brows);
    assert(bcols == 1);
    X = A\B;
    for i = 1:iters,
        residuals = A*X - B;
        w = calc_norm1_weights(residuals, dim);
        sA = scale_rows(w, A);
        sB = scale_rows(w, B);
        X = sA\sB;
    end
end
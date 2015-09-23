function W = distribute_weights(residuals)
    n = numel(residuals);
    Z = zeros(1, n-1);
    O = speye(n-1, n-1);
    P = [O; Z] - [Z; O];
    I = 1:n;
    W = sparse(I, I, sqrt(residuals), n, n);
    p = ones(n, 1);
    q = -(W*P)\(W*p);
    W = P*q + p;
end
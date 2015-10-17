function Ghat = apply_avg(G, dim)
    [rows, cols] = size(G);
    n = floor(rows/dim);
    K = kron(ones(n, 1), eye(dim));
    Ghat = G - (1/n)*K*(K'*G);
end
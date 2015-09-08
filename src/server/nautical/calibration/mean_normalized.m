function B = mean_normalized(A, dim)
    [m, n] = size(A);
    count = floor(m/dim);
    assert(count*dim == m);
    V = kron(normalize_vector(ones(count, 1)), eye(dim, dim));
    B = A - V*(V'*A);
end
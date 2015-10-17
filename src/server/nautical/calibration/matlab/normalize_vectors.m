function [F, J] = normalize_vectors(X, dim, mu)
    [rows, cols1] = size(X);
    n = floor(rows/dim);
    assert(dim*n == rows);
    
end
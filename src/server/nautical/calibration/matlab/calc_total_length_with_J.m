function [F, J] = calc_total_length_with_J(A, X, dim, mu)
    if nargin < 4,
        mu = 0.0001;
    end
    n = floor(size(A, 1)/dim);
    Y = A*X;
    lengths = calc_row_norms([get_array(Y, dim) mu*ones(n, 1)]);
    J = (Y./kron(lengths, [1 1]'))'*A;
    F = sum(lengths);
end
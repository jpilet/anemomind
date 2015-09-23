function [F, J] = normalize_by_total_length(A, X, dim, mu)
    if nargin < 4,
        mu = 0.0001;
    end
    [t, tJ] = calc_total_length_with_J(A, X, dim, mu);
    tInv = 1/t;
    tInvJ = -(1/t^2)*tJ;
    
    Y = A*X;
    
    F = tInv*Y;
    J = tInv*A + Y*tInvJ;
end
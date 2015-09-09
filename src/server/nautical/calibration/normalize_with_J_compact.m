function [F, J] = normalize_with_J_compact(X, DX)
    mu = 1.0e-4;
    l = norm([X; mu]);
    F = (1.0/l)*X;
    J = X*((-(X')./(l^3))*DX) + (1/l)*DX;
end
function [F, J] = normalize_with_J_many(X, dim)
    [xrows, xcols] = size(X);
    assert(mod(xrows, dim) == 0);
    assert(xcols == 1);
    F = zeros(xrows, 1);
    allJ = 
end
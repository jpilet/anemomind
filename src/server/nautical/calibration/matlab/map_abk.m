function [F, J] = map_abk(abk)
    assert(all(size(abk) == [3 1]));
    a = abk(1);
    b = abk(2);
    k = abk(3);
    F = [a b a*k b*k]';
    J = [1 0 0; 0 1 0; k 0 a; 0 k b];
end
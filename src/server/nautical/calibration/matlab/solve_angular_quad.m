function [min_angle, max_angle] = solve_angular_quad(Q)
    assert(all([3, 3] == size(Q)));
    a = Q(1, 1);
    b = Q(2, 2);
    %c = Q(3, 3);
    d = Q(2, 1);
    e = Q(3, 1);
    f = Q(2, 3);
    [C, phi] = rewrite_cos_sin_sum(-2*d, b-a);
    [D, theta] = rewrite_cos_sin_sum(2*f, -2*e);
end
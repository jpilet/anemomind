function E = calc_flow_constancy_errors(A, B, r)
    n = size(r, 1);
    E = nan(n, 4);
    for i = 1:n,
        r0 = r(i, :);
        R = range_to_higher_dim(r0(1):r0(2), 2);
        E(i, :) = calc_flow_constancy_error(A(R, :), B(R, :));
    end
end
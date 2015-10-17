function E = calc_flow_constancy_errors(A, B, r, settings)
    n = size(r, 1);
    E = nan(n, 2);
    for i = 1:n,
        r0 = r(i, :);
        R = range_to_higher_dim(r0(1):r0(2), 2);
        fprintf('Evaluate constancy error %d of %d\n', i, n);
        E(i, :) = calc_flow_constancy_error(A(R, :), B(R, :), settings);
    end
end
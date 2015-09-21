function [E, valid] = calc_flow_constancy_errors(A, B, r)
    n = size(r, 1);
    E = zeros(n, 1);
    valid = true(n, 1);
    for i = 1:n,
        r0 = r(i, :);
        R = range_to_higher_dim(r0(1):r0(2), 2);
        err = calc_flow_constancy_error(A(R, :), B(R, :));
        if isnan(err),
            fprintf('Failed to evaluate split %d of %d', i, n);
            E(i) = nan;
            valid(i) = false;
        else
            E(i) = err;
        end
    end
end
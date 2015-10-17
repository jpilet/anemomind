function true_flows = compute_local_flows(A, B, ranges, settings)
    n = size(ranges, 1);
    assert(size(ranges, 2) == 2);
    true_flows = zeros(n, 2);
    for i = 1:n,
        r = make_range_from_endpoints(ranges(i, :));
        r2 = range_to_higher_dim(r, 2);
        f = compute_local_flow(A(r2, :), B(r2, :), settings);
        true_flows(i, :) = f;
    end
end
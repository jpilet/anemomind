function [Yest, angles] = minimize_flow_subspace_dists(Q, B, settings)
    [n2, dims] = size(Q);
    assert(size(B, 1) == n2);
    n = get_observation_count(Q);
    Yest = zeros(settings.count, dims);
    angles = zeros(settings.count, 1);
    for i = 1:settings.count,
        r2 = range_to_higher_dim(sample_range(n, settings.min_length, settings.max_length), 2);
        [Yest(i, :), angles(i)] = minimize_flow_subspace_dist(Q(r2, :), B(r2, :));
    end
    good = isfinite(angles);
    Yest = Yest(good, :);
    angles = angles(good);
end
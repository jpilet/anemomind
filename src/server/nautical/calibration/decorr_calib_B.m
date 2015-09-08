function params = decorr_calib_B(A, B, chunk_size)
    basis = [A B];
    n = get_observation_count(A);
    ranges = make_ranges(n, chunk_size);
    range_count = size(ranges, 1);
    Q = zeros(range_count, size(basis, 2));
    for i = 1:range_count,
        r = get_range_from_R(ranges, i);
        Q(i, :) = normalize_vector(mean_normalized(B(r, :), 2))'*basis(r, :);
    end
    params = -Q(:, 1:(end-1))\Q(:, end);
end
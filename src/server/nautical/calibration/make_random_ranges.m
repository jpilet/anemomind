function R = make_random_ranges(count, n, max_size)
    R = zeros(count, 2);
    for i = 1:count,
        R(i, :) = make_random_endpts(n, max_size);
    end
end
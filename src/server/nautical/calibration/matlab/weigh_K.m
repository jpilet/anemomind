function wK = weigh_K(data, weights)
    n = numel(weights);
    assert(numel(data.out_ranges) == n);
    wK = zeros(size(data.K));
    for i = 1:n,
        r = data.out_ranges(i).data;
        wK(r, :) = weights(i)*data.K(r, :);
    end
end
function R = make_ranges(n, chunk_size, overlap)
    if nargin < 3,
        overlap = 0.1;
    end
    lambda = overlap;
    from = 1:ceil(lambda*chunk_size):n;
    from = from(:);
    to = from + chunk_size-1;
    keep = to <= n;
    R = [from(keep) to(keep)];
end
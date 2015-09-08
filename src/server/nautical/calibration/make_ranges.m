function R = make_ranges(n, chunk_size)
    lambda = 0.1;
    from = 1:floor(lambda*chunk_size):n;
    from = from(:);
    to = from + chunk_size-1;
    keep = to <= n;
    R = [from(keep) to(keep)];
end
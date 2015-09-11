function [r, durs] = get_ranges(ds, thresh_seconds)
    difs = ds(2:end, 1) - ds(1:(end-1), 1);
    split = find(difs > thresh_seconds);
    from = [1; split];
    n = size(ds, 1);
    to = [split+1; n];
    r = [from(:) to(:)];
    durs = difs(split);
end
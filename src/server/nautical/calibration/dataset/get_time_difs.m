function from_to_dur = get_time_difs(ds)
    n = size(ds, 1);
    from = (1:(n-1))';
    to = from + 1;
    durs = ds(to, 1) - ds(from, 1);
    from_to_dur = sortrows([from to durs], -3);
end
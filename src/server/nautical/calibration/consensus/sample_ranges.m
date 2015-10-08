function ranges = sample_ranges(n, minl, maxl, range_count)
    ranges = make_array(range_count);
    for i = 1:range_count,
        ranges(i).data = sample_range(n, minl, maxl);
    end
end
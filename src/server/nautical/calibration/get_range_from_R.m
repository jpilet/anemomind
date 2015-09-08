function r = get_range_from_R(R, i)
    first = R(i, 1);
    last = R(i, 2);
    from = 2*(first - 1)+1;
    to = from + 2*(last - first + 1) - 1;
    r = from:to;
    assert(mod(numel(r), 2) == 0);
end
function r = sample_range(n, min_length, max_length)
    len = round(min_length + rand(1, 1)*(max_length - min_length));
    from = randi(n-len+1);
    to = from + len - 1;
    r = from:to;
end
function from_to = make_random_endpts(n, max_size)
    %from_to = sort(randi(n, 1, 2));
    size = randi(max_size, 1, 1);
    %size = max_size;
    from = randi(n - size, 1, 1);
    to = from + size;
    from_to = [from to];
end
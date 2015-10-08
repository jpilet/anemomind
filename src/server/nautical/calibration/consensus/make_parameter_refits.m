function refits = make_parameter_refits(A, B, settings)
    Q = gram_schmidt(A);
    R = Q'*A;
    refits = make_array(settings.count);
    n = get_observation_count(A);
    for i = 1:settings.count,
        len = round(settings.min_length + rand(1, 1)*(settings.max_length - settings.min_length));
        from = randi(n-len+1);
        to = from + len - 1;
        r = from:to;
        r2 = range_to_higher_dim(r, 2);
        assert(mod(numel(r2), 2) == 0);
        refits(i).data = make_parameter_refit(Q(r2, :), B(r2, :), R);
    end
end
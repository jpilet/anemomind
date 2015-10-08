function refits = make_parameter_refits(A, B, settings)
    Q = gram_schmidt(A);
    R = Q'*A;
    refits = make_array(settings.count);
    n = get_observation_count(A);
    for i = 1:settings.count,
        if false,
            r = sample_range(n, settings.min_length, settings.max_length);
            r2 = range_to_higher_dim(r, 2);
            assert(mod(numel(r2), 2) == 0);
            refits(i).data = make_parameter_refit(Q(r2, :), B(r2, :), R);
        else
            refits(i).data = sample_parameter_refit(Q, B, R, n, settings.min_length, settings.max_length, settings.spans);
        end
    end
end
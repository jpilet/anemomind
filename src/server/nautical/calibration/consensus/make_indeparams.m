function [indeparams, R] = make_indeparams(A, B, settings)
    Q = gram_schmidt(A);
    R = Q'*A;
    n = get_observation_count(A);
    indeparams = make_array(settings.count);
    for i = 1:settings.count,
        r = sample_range(n, settings.min_length, settings.max_length);
        r2 = range_to_higher_dim(r, 2);
        indeparams(i).data = make_indeparam(Q(r2, :), B(r2, :));
    end
end
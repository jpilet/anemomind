function optimizers = make_indeparam_optimizers(A, B, settings)
    n = get_observation_count(A);
    optimizers = make_array(settings.count);
    for i = 1:settings.count,
        r = sample_range(n, settings.min_length, settings.max_length);
        r2 = range_to_higher_dim(r, 2);
        optimizers(i).data = make_indeparam_optimizer(A(r2, :), B(r2, :));
    end
end
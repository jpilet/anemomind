function params = decorr_calib(Aw, Bw, Ac, Bc, chunk_size)
    homo = false;

    if homo,
        basis = gram_schmidt([Aw Bw]);
    else
        basis = [Aw Bw];
    end
    
    n = get_observation_count(Aw);
    R = make_ranges(n, chunk_size);
    range_count = size(R, 1);
    r = get_range_from_R(R, 1);
    [out_coefs, in_coefs] = size(make_linear_fitness(Ac(r, :), Bc(r, :), basis(r, :)));
    C = zeros(out_coefs*range_count, in_coefs);
    for i = 1:range_count,
        cRange = get_range(i, out_coefs);
        r = get_range_from_R(R, i);
        C(cRange, :) = make_linear_fitness(Ac(r, :), Bc(r, :), basis(r, :));
    end
    if homo,
        params_ = calc_smallest_eigvec(C'*C);
        params = recover_scaled_params(Aw, Bw, basis*params_);
    else
        params = -C(:, 1:(end-1))\C(:, end);
    end
end


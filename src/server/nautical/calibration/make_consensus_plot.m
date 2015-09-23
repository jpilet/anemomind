function make_consensus_plot(A_, B)
    A = A_(:, 1:2);
    n = get_observation_count(A);
    count = 30000;
    pts = zeros(count, 2);
    valid = false(count, 1);
    for i = 1:count,
        if mod(i, 100) == 0,
            fprintf('Computing point %d of %d\n', i, count);
        end
        from_to = sort(randi(n, 1, 2));
        r = range_to_higher_dim(make_range_from_endpoints(from_to), 2);
        params = fit_params(A(r, :), B(r, :));
        if ~isempty(params),
            pts(i, :) = params;
            valid(i) = true;
        end
    end
    plotx(pts(valid, :), '.k', 'MarkerSize', 12);
    axis equal;
end

function params = fit_params(A, B)
    Q = gram_schmidt(A(:, 1:2));
    R = Q'*A;
    [K, opt] = make_fitness(-B, Q);
    try
        p = calc_smallest_eigvec(K'*K);
    catch e,
        params = [];
        return
    end
    scale = opt(1, :)*p;
    params = (1/scale)*(R\p);
end
function params = optimize_trajectory_smoothness5(A, B, block_size, visualize)
    %[A, B] = normalize_by_gps(A, B);

    order = 2;
    lambda = 160000;
    
    cumulativeAB = cumulative_row_sum([A B], 2);
    Qab = make_smooth(cumulativeAB, 2, order, lambda) - cumulativeAB;
    Qa = Qab(:, 1:4);
    Qb = Qab(:, 5);
    
    k = 0;
    ab = [1; 0];
    iters = 30;
    for i = 1:iters,
        k = solve_for_k(Qa, Qb, ab)
        ab = solve_for_ab(A, B, k, order, lambda)
    end
    
    
    [params, scale] = recover_scaled_params(A, B, basis*X);
    dc = zeros(size(A, 2), 1);
    dc(1) = 1;
    default_flow = (A*dc + B);
    default_coefs = (basis\default_flow);
    sX = (1/scale)*X;
    if visualize,
        params
        plotx(get_array(rowsum*default_coefs, 2), 'k');
        calibrated = get_array(rowsum*sX, 2);
        hold on
        plotx(get_array(smooth*sX, 2), 'r');
        plotx(calibrated, 'b');
        text(calibrated(end, 1), calibrated(end, 2), sprintf('Calibrated (%d)', numel(params)));
        hold off
        axis equal
    end
end

function k = solve_for_k(Qa, Qb, ab)
    assert(size(Qa, 2) == 4);
    assert(size(Qb, 2) == 1);
    k = (Qa(:, 3:4)*ab)\(-Qb - Qa(:, 1:2)*ab);
end

function ab = solve_for_ab(A, B, k, order, lambda)
    P = A(:, 1:2) + k*A(:, 3:4);
    basis = gram_schmidt([P B]);
    C = cumulative_row_sum(basis, 2);
    Q = make_smooth(C, 2, order, lambda) - C;
    params = calc_smallest_eigvec(Q'*Q);
    ab = recover_scaled_params(P, B, basis*params);
end
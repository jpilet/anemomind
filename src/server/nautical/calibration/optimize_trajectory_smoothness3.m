function params = optimize_trajectory_smoothness3(A, B, block_size, visualize)
    norm1 = false;
    order = 2;
    lambda = 160000;
    
    basis1 = gram_schmidt([A(:, 1:2) B]);
    basis2 = A(:, 3:end);
    basis = [basis1 basis2];
    main = 1:3;
    rest = 4:size(basis, 2);
    
    rowsum = cumulative_row_sum(basis, 2);
    smooth = make_smooth(rowsum, 2, order, lambda);
    Qfull = smooth - rowsum;
    Qmain = Qfull(:, main);
    Qrest = Qfull(:, rest);
    restOpt = -Qrest\Qmain;
    Q = Qmain + Qrest*restOpt;
    
    if norm1,
        Xmain = minimize_norm1_homogeneous(Q, 2);
    else
        Xmain = calc_smallest_eigvec(Q'*Q);
    end
    X = [Xmain; restOpt*Xmain];

    
    [params, scale] = recover_scaled_params(A, B, basis*X);
    dc = zeros(size(A, 2), 1);
    dc(1) = 1;
    default_flow = (A*dc + B);
    default_coefs = (basis\default_flow);
    sX = (1/scale)*X;
    if visualize,
        params
        plotx(get_array(rowsum*sX, 2), 'b');
        hold on
        plotx(get_array(smooth*sX, 2), 'r');
        plotx(get_array(rowsum*default_coefs, 2), 'k');
        hold off
    end
end
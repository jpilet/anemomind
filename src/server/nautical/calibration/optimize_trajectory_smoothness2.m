function params = optimize_trajectory_smoothness2(A, B, block_size, visualize)
    homo = true;
    norm1 = false;
    order = 2;
    lambda = 160000;
    
    if homo,
        basis = gram_schmidt([A B]);
    else
        basis = [A B];
    end
    rowsum = cumulative_row_sum(basis, 2);
    smooth = make_smooth(rowsum, 2, order, lambda);
    Q = smooth - rowsum;
    
    if homo,
        if norm1,
            X = minimize_norm1_homogeneous(Q, 2);
        else
            X = calc_smallest_eigvec(Q'*Q);
        end
    else
        if norm1,
            X = [minimize_norm1(Q(:, 1:(end-1)), -Q(:, end), 2); 1];
        else
            X = -Q(:, 1:(end-1))\Q(:, end);
        end
    end
    
    
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
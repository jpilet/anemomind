function params = optimize_trajectory_smoothness(A, B, block_size, visualize)
    homo = true;
    norm1 = false;
    order = 2;
    
    if homo,
        basis = gram_schmidt([A B]);
    else
        basis = [A B];
    end
    rowsum = cumulative_row_sum(basis, 2);
    [blocks, block_count] = calc_avg_blocks(rowsum, block_size, 2);
    R = kron(make_difs_sparse(block_count, order), speye(2));
    Q = R*blocks;
    
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
    
    params = recover_scaled_params(A, B, basis*X);
    if visualize,
        params
        plotx(get_array(rowsum*X, 2), 'b');
        hold on
        plotx(get_array(blocks*X, 2), 'xr');
        hold off
    end
end
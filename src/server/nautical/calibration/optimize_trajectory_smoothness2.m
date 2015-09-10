function [params, rowsum] = optimize_trajectory_smoothness2(A, B, block_size, visualize)
    %[A, B] = normalize_by_gps(A, B);

    homo = true;
    norm1 = false;
    order = 2;
    lambda = 160000;
    
    if homo,
        basis = gram_schmidt([A B]);
        %basis = normalize_rows([A B]')';
    else
        basis = [A B];
    end
    rowsum = cumulative_row_sum(basis, 2);
    smooth = make_smooth(rowsum, 2, order, lambda);
    Q = smooth - rowsum;
    %[qrows, qcols] = size(Q);
    %qeqs = floor(qrows/2);
    %Q = kron(make_difs_sparse(qeqs, 1), speye(2, 2))*Q;
    fprintf('Condition of Q: %.9g', cond(Q));
    if homo,
        if norm1,
            X = minimize_norm1_homogeneous(Q, 2);
        else
            QtQ = Q'*Q;
            [V, D] = eig(QtQ);
            figure;
            plot(diag(D));
            X = calc_smallest_eigvec(QtQ);
        end
    else
        if norm1,
            X = [minimize_norm1(Q(:, 1:(end-1)), -Q(:, end), 2); 1];
        else
            X = [-Q(:, 1:(end-1))\Q(:, end); 1];
        end
    end
    
    
    [params, scale] = recover_scaled_params(A, B, basis*X);
    dc = zeros(size(A, 2), 1);
    dc(1) = 1;
    default_flow = (A*dc + B);
    default_coefs = (basis\default_flow);
    sX = (1/scale)*X;
    if visualize,
        figure;
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
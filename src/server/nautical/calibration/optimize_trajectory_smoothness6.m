function [params, rowsum] = optimize_trajectory_smoothness6(A, B, block_size, visualize)
    norm1 = false;
    order = 2;
    lambda = 160000;
    
    basis = gram_schmidt([gram_schmidt(A) B]);
    %basis = [A B];
    
    rowsum = cumulative_row_sum(basis, 2);
    smooth = make_smooth(rowsum, 2, order, lambda);
    Q = smooth - rowsum;
    Qparam = Q(:, 1:(end-1));
    Qgps = Q(:, end);
    gpsOpt = -Qgps\Qparam;
    
    Qtot = Qparam + Qgps*gpsOpt;
    opt_params = calc_smallest_eigvec(Qtot'*Qtot);
    tot_params = [opt_params; gpsOpt*opt_params];
    
    
    [params, scale] = recover_scaled_params(A, B, basis*tot_params);
    
    dc = zeros(size(A, 2), 1);
    dc(1) = 1;
    default_flow = (A*dc + B);
    default_coefs = (basis\default_flow);
    As = cumulative_row_sum(A, 2);
    Bs = cumulative_row_sum(B, 2);
    if visualize,
        params
        plotx(get_array(rowsum*default_coefs, 2), 'k');
        calibrated = As*params + Bs;
        c2 = get_array(calibrated, 2);
        hold on
        plotx(get_array(make_smooth(calibrated, 2, order, lambda), 2), 'r');
        plotx(c2, 'b');
        text(c2(end, 1), c2(end, 2), sprintf('Calibrated (%d)', numel(params)));
        hold off
        axis equal
    end
end
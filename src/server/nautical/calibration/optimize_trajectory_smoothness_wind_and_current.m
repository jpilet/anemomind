function params = optimize_trajectory_smoothness_wind_and_current(Aw, Bw, Ac, Bc)
    order = 2;
    lambda = 160000;
    [arows, acols] = size(Aw);
    assert(all([arows, acols] == size(Ac)));
    assert(norm(Bw - Bc) < 1.0e-9);
    A = [Aw zeros(arows, acols); zeros(arows, acols) Ac];
    B = [Bw; Bc];
    basis = gram_schmidt([A B]);
    basis_w = cumulative_row_sum(basis(1:arows, :), 2);
    basis_ws = make_smooth(basis_w, 2, order, lambda);
    basis_c = cumulative_row_sum(basis((arows+1):end, :), 2);
    basis_cs = make_smooth(basis_c, 2, order, lambda);
    Q = [basis_ws - basis_w; ...
         basis_cs - basis_c];
    params0 = calc_smallest_eigvec(Q'*Q);
    [params, scale] = recover_scaled_params(A, B, basis*params0);
    f = 1.0/scale;
    
    d = [1 0 0 0]';
    
    visualize = true;
    if visualize,
        subplot(1, 2, 1);
        plot_with_smooth(basis_w, basis_ws, f*params0);
        hold on
        plotx(get_array(cumulative_row_sum(Aw*d + Bw, 2), 2), 'k');
        hold off
        
        subplot(1, 2, 2);
        plot_with_smooth(basis_c, basis_cs, f*params0);
        hold on
        plotx(get_array(cumulative_row_sum(Ac*d + Bc, 2), 2), 'k');
        hold off
    end
end

function plot_with_smooth(b, bs, p)
    X = get_array(b*p, 2);
    Xs = get_array(bs*p, 2);
    plotx(X, 'b');
    hold on
    plotx(Xs, 'r');
    hold off
end
function params = calibrate_straightest(A, B, settings)
    if nargin < 3,
        settings = make_straightest_settings();
    end
    AB = apply_avg([A B], 2);
    ABcum = cumulative_row_sum(AB, 2);
    difAB = make_sliding_difs(size(ABcum, 1) - 2*settings.window_size, size(ABcum, 1))*ABcum;
    Q = gram_schmidt(difAB);
    R = Q'*difAB;
    edges = AB/R;
    
    params0 = calc_smallest_eigvec(edges'*edges);
    %params0 = minimize_norm1_homogeneous(edges, 2, 30);
    
    params1h = R\params0;
    params1 = (1.0/params1h(end))*params1h;
    params = params1(1:(end-1));
    default_params = [1 0 0 0 1]';
    
    if settings.visualize,
        plotx(get_array(ABcum*params1, 2));
        hold on
        plotx(get_array(ABcum*default_params, 2), 'k');
        hold off
        axis equal;
    end
end
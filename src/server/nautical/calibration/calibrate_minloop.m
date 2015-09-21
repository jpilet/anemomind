function params = calibrate_minloop(A, B, settings)
    if nargin < 3,
        settings = make_minloop_settings();
    end
    AB = [A B];
    [rows, cols] = size(AB);
    n = floor(rows/2);
    Q = gram_schmidt(AB);
    R = Q'*AB;
    
    
    if false,
        ranges = make_ranges(n, settings.chunk_size, settings.overlap);
        sizes = ranges(:, 2) - ranges(:, 1) + 1;

        d_rows = 2*sum(sizes);
        D = zeros(d_rows, cols);
        from = 0;
        for i = 1:size(ranges, 1);
            r = ranges(i, :);
            src_r = range_to_higher_dim(r(1):r(2), 2);
            dst_r = from + src_r;

            D(dst_r, :) = calc_cost(Q(src_r, :));

            from = dst_r(end);
        end
        D = [D; calc_cost(Q)];
    end
    
    D = calc_cost(Q);
    fprintf('Number of rows: %d\n', size(D));
    
    
    avg_cum = cumulative_row_sum(D, 2);
    Q_cum = cumulative_row_sum(Q, 2);
    if settings.norm_type == 2,
        K = D'*D;
        params0 = calc_smallest_eigvec(K);        
    elseif settings.norm_type == 1,
        [params0, K] = minimize_norm1_homogeneous(D, 2, 30);
    else
        error('Bad norm type.');
    end
    if settings.visualize,
        [~, D] = eig(K);
        plot(diag(D));
    end

    
    params1 = R\params0;
    params = (1.0/params1(end))*params1(1:(end-1));
    
    default_params = zeros(size(params));
    default_params(1) = 1;
    default_params1 = [default_params; 1];
    default_params0 = normalize_vector(R*default_params1)
    
    if norm(default_params0 - params0) > norm(default_params0 + params0),
        default_params0 = -default_params0;
    end
    
    if settings.visualize,
        subplot(1, 2, 1);
        plotx(get_array(avg_cum*params0, 2), 'b');
        hold on
        plotx(get_array(avg_cum*default_params0, 2), 'k');
        hold off
        axis equal;
        
        subplot(1, 2, 2);
        plotx(get_array(Q_cum*params0, 2), 'b');
        hold on
        plotx(get_array(Q_cum*default_params0, 2), 'k');
        hold off        
        axis equal;
    end
end


function C = calc_cost0(A)
    C = cumulative_row_sum(apply_avg(A, 2), 2);
end

function C = calc_cost(A)
    C = apply_avg(A, 2);
end
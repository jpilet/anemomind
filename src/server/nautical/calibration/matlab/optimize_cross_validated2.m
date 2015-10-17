function params = optimize_cross_validated2(A, B, ranges0, settings)
    ranges = make_even_rows(ranges0);
    data = make_local_flow_matrix(A, B, ranges);
    range_count = size(ranges, 1);
    
    split_size = range_count/2;
    
    split_A = logical(repmat([1 0]', split_size, 1));
    split_B = ~split_A;
    
    weights = ones(range_count, 1);
    for i = 1:settings.iters,
        wK = weigh_K(data, weights);
        params0_bad_scale = calc_smallest_eigvec(wK'*wK);
        params0 = (1/(data.opt_scale*params0_bad_scale))*params0_bad_scale;
        flows = get_array(data.opt_flows*params0, 2);
        
        difs = flows(split_A, :) - flows(split_B, :);
        difs(isnan(difs)) = 1.0e9;
        residuals = calc_row_norms(difs);
        residuals(residuals < settings.thresh) = settings.thresh;
        weights0 = distribute_weights(residuals);
        weights = kron(weights0(:), [1; 1]);
        
        if settings.visualize,
            subplot(1, 2, 1);
            sorted_residuals = sort(residuals);
            m = floor(0.5*numel(sorted_residuals));
            plot(sorted_residuals(1:m));
            
            subplot(1, 2, 2);
            plotx(v1(flows));
            title(sprintf('ITERATION %d of %d', i, settings.iters));
            drawnow;
            pause(settings.visualize);
        end
    end
    params = data.R\params0;
end

function Y = v1(X)
    Y = cumulative_row_sum(X, 1);
end

function Y = make_even_rows(X)
    n = floor(size(X, 1)/2);
    rows = 2*n;
    Y = X(1:rows, :);
end
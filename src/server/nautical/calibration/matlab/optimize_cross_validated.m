function params = optimize_cross_validated(A, B, ranges, settings)
    local_flows = compute_local_flows(A, B, ranges, settings);
    data = make_local_flow_matrix(A, B, ranges);
    range_count = size(ranges, 1);
    weights = ones(range_count, 1);
    for i = 1:settings.iters,
        wK = weigh_K(data, weights);
        params0_bad_scale = calc_smallest_eigvec(wK'*wK);
        params0 = (1/(data.opt_scale*params0_bad_scale))*params0_bad_scale;
        flows = get_array(data.opt_flows*params0, 2);
        difs = flows - local_flows;
        difs(~isfinite(difs)) = 1.0e9;
        residuals = calc_row_norms(difs);
        residuals(residuals < settings.thresh) = settings.thresh;
        weights = distribute_weights(residuals);
        
        if settings.visualize,
            subplot(1, 2, 1);
            sorted_residuals = sort(residuals);
            m = floor(0.1*numel(sorted_residuals));
            plot(sorted_residuals(1:m));
            
            subplot(1, 2, 2);
            plotx(v1(flows));
            hold on
            plotx(v1(local_flows));
            hold off
            
            drawnow;
            pause(settings.visualize);
        end
    end
    params = data.R\params0;
end

function Y = v1(X)
    Y = cumulative_row_sum(X, 1);
end


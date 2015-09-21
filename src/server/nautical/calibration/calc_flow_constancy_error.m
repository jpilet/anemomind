function E = calc_flow_constancy_error(A, B, settings)
    n = get_observation_count(A);
    assert(n == get_observation_count(B));
    
    % Cumulative winds.
    sp = round(make_linear_table(5, 0, n));
    
    R0 = [(sp(1)+1):sp(2) (sp(3)+1):sp(4)];
    R1 = [(sp(2)+1):sp(3) (sp(4)+1):sp(5)];
    Rfull = (sp(1)+1):sp(5);
    assert(Rfull(end) == n);
    
    visualize = true;
    E = nan*ones(1, 4);
    
    if settings.common_scale,
        [F0, F1] = reconstruct_for_splits(A, B, R0, R1);
    else
        F0 = reconstruct_for_split(A, B, R0);
        F1 = reconstruct_for_split(A, B, R1);
    end
    E = avg_error(F0 - F1);

    if visualize,
        plotx(get_array(F0(:, 2), 2));
        hold on
        plotx(get_array(F1(:, 2), 2), 'r');
        hold off
        drawnow;
        pause(0.5);
    end
end

function err_per_col = avg_error(difs)
    n = get_observation_count(difs);
    err_per_col = sqrt((1.0/n)*sum(difs.^2));
end


function [F0, F1] = reconstruct_for_splits(A, B, R0, R1)
    Q = gram_schmidt(A);
    R = Q'*A;
    n = get_observation_count(A);
    lhs = cumulative_row_sum([-B make_subset_ones(n, R0) make_subset_ones(n, R1)], 2);
    rhs = cumulative_row_sum(Q, 2);
    opt = lhs\rhs;
    K = lhs*opt - rhs;
    try
        params0 = calc_smallest_eigvec(K'*K);
    catch e,
        params0 = nan*ones(4, 1);
    end
    scale_and_flows = opt*params0;
    scale = scale_and_flows(1);
    true_params = (1/scale)*scale_and_flows;
    tw0 = repmat(true_params(2:3, :), n, 1);
    tw1 = repmat(true_params(4:5, :), n, 1);
    F0 = speed_and_trajectory(tw0-B);
    F1 = speed_and_trajectory(tw1-B);
end

function out = make_subset_ones(n, R)
    out = zeros(n, 1);
    out(R) = 1;
    out = kron(out, eye(2, 2));
end

function E = reconstruct_for_split(Afull, Bfull, R)
    n = numel(R);
    R2 = range_to_higher_dim(R, 2);
    
    A = Afull(R2, :);
    B = Bfull(R2, :);
    
    size(A, 1);
        
    Q = gram_schmidt(A);
    apparent_wind = [-B, kron(ones(n, 1), eye(2, 2))];
    
    lhs = cumulative_row_sum(apparent_wind, 2);
    rhs = cumulative_row_sum(Q, 2);
    
    %lhs = apparent_wind;
    %rhs = Q;
    
    opt = (lhs\rhs);
    K = lhs*opt - rhs;
    try,
        params0 = calc_smallest_eigvec(K'*K);
    catch e,
        params0 = nan*ones(4, 1);
    end
    scale_and_scaled_true_wind = opt*params0;
    scale = scale_and_scaled_true_wind(1);
    params = (1/scale)*params0;
    true_params = A\(Q*params);
    E = speed_and_trajectory(Afull*true_params);
end

function Y = speed_and_trajectory(X)
    Y = [X acc2(X)];
end



function Y = acc2(X)
    Y = cumulative_row_sum(X, 2);
    Y = Y(3:end, :);
    assert(size(Y, 1) == size(X, 1));
end
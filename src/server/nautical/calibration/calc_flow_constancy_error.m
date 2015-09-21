function E = calc_flow_constancy_error(A, B)
    n = get_observation_count(A);
    assert(n == get_observation_count(B));
    
    % Cumulative winds.
    sp = round(make_linear_table(5, 0, n));
    
    R0 = [(sp(1)+1):sp(2) (sp(3)+1):sp(4)];
    R1 = [(sp(2)+1):sp(3) (sp(4)+1):sp(5)];
    Rfull = (sp(1)+1):sp(5);
    try
        [tw0, aw0] = reconstruct_for_split(A, B, R0, Rfull);
        [tw1, aw1] = reconstruct_for_split(A, B, R1, Rfull);
        difs_aw = aw0 - aw1;
        difs_tw = tw0 - tw1;
        difs = difs_tw;
        E = sqrt(norm(difs)^2/numel(Rfull));
        assert(isscalar(E));
    catch e,
        E = nan;
    end
end

function [tw, aw] = reconstruct_for_split(Afull, Bfull, R, Rfull)
    n = numel(R);
    R2 = range_to_higher_dim(R, 2);
    
    A = Afull(R2, :);
    B = Bfull(R2, :);
    
    size(A, 1);
        
    Q = gram_schmidt(A);
    apparent_wind = [-B, kron(ones(n, 1), eye(2, 2))];
    
    lhs = cumulative_row_sum(apparent_wind, 2);
    rhs = cumulative_row_sum(Q, 2);
    opt = (lhs\rhs);
    K = lhs*opt - rhs;
    params0 = calc_smallest_eigvec(K'*K);
    scale_and_scaled_true_wind = opt*params0;
    scale = scale_and_scaled_true_wind(1);
    params = (1/scale)*params0;

    R2full = range_to_higher_dim(Rfull, 2);
    
    cumulative = false;
    
    if cumulative,
        Ac = cumulative_row_sum(Afull(R2full, :), 2);
        Bc = cumulative_row_sum(Bfull(R2full, :), 2);
    else
        Ac = Afull(R2full, :);
        Bc = Bfull(R2full, :);
    end
    
    true_params = A\(Q*params);
    aw = Ac*true_params;
    tw = aw + Bc;
end


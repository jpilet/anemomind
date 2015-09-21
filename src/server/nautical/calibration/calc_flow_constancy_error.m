function E = calc_flow_constancy_error(A, B)
    n = get_observation_count(A);
    assert(n == get_observation_count(B));
    
    % Cumulative winds.
    sp = round(make_linear_table(5, 0, n));
    
    R0 = [(sp(1)+1):sp(2) (sp(3)+1):sp(4)];
    R1 = [(sp(2)+1):sp(3) (sp(4)+1):sp(5)];
    Rfull = (sp(1)+1):sp(5);
    assert(Rfull(end) == n);
    
    try
        err0 = reconstruct_for_split(A, B, R0);
        err1 = reconstruct_for_split(A, B, R1);
        E = avg_error(err0 - err1);
    catch e,
        E = nan*ones(1, 4);
    end
end

function err_per_col = avg_error(difs)
    n = get_observation_count(difs);
    err_per_col = sqrt((1.0/n)*sum(difs.^2));
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
    opt = (lhs\rhs);
    K = lhs*opt - rhs;
    params0 = calc_smallest_eigvec(K'*K);
    scale_and_scaled_true_wind = opt*params0;
    scale = scale_and_scaled_true_wind(1);
    params = (1/scale)*params0;

    Ac = acc2(Afull);
    Bc = acc2(Bfull);
    
    true_params = A\(Q*params);
    E = [Afull*true_params+Bfull Afull*true_params Ac*true_params+Bc Ac*true_params];
end

function Y = acc2(X)
    Y = cumulative_row_sum(X, 2);
    Y = Y(3:end, :);
    assert(size(Y, 1) == size(X, 1));
end
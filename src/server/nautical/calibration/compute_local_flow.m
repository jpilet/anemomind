function flow = compute_local_flow(A, B, settings)
    rhs = gram_schmidt(A);
    n = get_observation_count(A);
    lhs = [-B kron(ones(n, 1), eye(2, 2))];
    
    opt = lhs\rhs;
    F = lhs*opt - rhs;
    try
        params0 = calc_smallest_eigvec(F'*F);
    catch e,
        params0 = nan*[1 1 1 1]';
    end
    scale_and_flow = opt*params0;
    flow = (1.0/scale_and_flow(1))*scale_and_flow(2:3);
end
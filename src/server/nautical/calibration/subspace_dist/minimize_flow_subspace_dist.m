function [X, angle] = minimize_flow_subspace_dist(Q, B)
    n = get_observation_count(Q);
    W = [-B kron(ones(n, 1), eye(2))];
    [w_coefs, q_coefs, angle] = minimize_subspace_dist(W, Q);
    X = (1.0/w_coefs(1))*q_coefs;
end
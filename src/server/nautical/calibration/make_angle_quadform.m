function Q = make_angle_quadform(A2, B)
    n = get_observation_count(A2);
    lhs = [-B kron(ones(n, 1), eye(2, 2))];
    K = make_fitness(lhs, A2);
    Q = K'*K;
end
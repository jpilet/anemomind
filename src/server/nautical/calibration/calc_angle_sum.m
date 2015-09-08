function s = calc_angle_sum(flows)
    n = get_observation_count(flows);
    D = kron(make_difs_sparse(n, 1), speye(2, 2));
    difs = get_array(D*flows, 2);
    angles = atan2(difs(:, 2), difs(:, 1));
    s = sum(abs(angles).^2);
end

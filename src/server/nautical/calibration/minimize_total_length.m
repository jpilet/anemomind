function true_params = minimize_total_length(A, B)
    basis = gram_schmidt([A B]);
    n = size(basis, 1)/2;
    Q = kron(make_difs_sparse(n, 3), eye(2))*basis;
    params = minimize_dist2d_subj_to_homog(Q);
    %params = calc_smallest_eigvec(Q'*Q);
    flow = basis*params;
    true_params = recover_scaled_params(A, B, flow);
end
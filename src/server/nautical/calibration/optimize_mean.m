function params = optimize_mean(A, B)
    basis = gram_schmidt([A B]);
    G = mean_normalized(basis, 2);
    p = calc_smallest_eigvec(G'*G);
    params = recover_scaled_params(A, B, basis*p);
end
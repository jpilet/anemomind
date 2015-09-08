function B = make_linear_fitness(A, B, basis)
    G = gram_schmidt(mean_normalized([A B], 2));
    B = G\mean_normalized(basis, 2);
end
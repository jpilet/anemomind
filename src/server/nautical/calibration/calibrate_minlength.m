function params = calibrate_minlength(A, B)
    Q = gram_schmidt(A);
    R = Q'*A;
    [F, opt] = make_fitness(-B, Q);
    params0 = calc_smallest_eigvec(F'*F);
    scale = opt(1, :)*params0;
    params = (1/scale)*(R\params0);
end
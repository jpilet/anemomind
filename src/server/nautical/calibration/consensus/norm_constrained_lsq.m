function X = norm_constrained_lsq2(A, B)
    assert(size(A, 1) == size(B, 1));
    assert(size(A, 2) == 2);
    assert(size(B, 2) == 1);
    C = A\B;
    [V, D] = eig(A'*A);
    plane = V(:, 1:(end-1));
    main = V(:, end);
    proj = C - main*dot(main, C);
    norm_proj = norm(proj);
    if norm_proj > 1,
        X = (1/norm_proj)*proj;
    else
        l = sqrt(1 - norm_proj^2);
        Xa = proj + l*main;
        Xb = proj - l*main;
        if norm(A*Xa - B) < norm(A*Xb - B),
            X = Xa;
        else
            X = Xb;
        end
    end
end
function [a_coefs, b_coefs, angle] = minimize_subspace_dist(A, B)
    full_space = [A B];
    Qfull = gram_schmidt(full_space);
    [Adist, Ap] = make_proj_dist(A, Qfull);
    [Bdist, Bp] = make_proj_dist(B, Qfull);
    K = [Adist; Bdist];
    KtK = K'*K;
    if all(isfinite(KtK(:))),
        full_coefs = calc_smallest_eigvec(K'*K);
        a_coefs = Ap*full_coefs;
        b_coefs = Bp*full_coefs;
        a_vec = A*a_coefs;
        b_vec = B*b_coefs;
        angle = acos(dot(a_vec, b_vec)/(norm(a_vec)*norm(b_vec)));
    else
        a_dims = size(A, 2);
        b_dims = size(B, 2);
        a_coefs = nan(a_dims, 1);
        b_coefs = nan(b_dims, 1);
        angle = nan;
    end
end

function [D, AP] = make_proj_dist(A, P)
    AP = A\P;
    D = A*AP - P;
end
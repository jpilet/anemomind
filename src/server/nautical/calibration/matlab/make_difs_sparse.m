function difs = make_difs_sparse(n, order)
    assert(order > 0);
    if order == 1,
        difs = make_small_difs(n);
    else
        K = make_difs_sparse(n, order-1);
        difs = make_small_difs(size(K, 1))*K;
    end
end

function X = make_small_difs(n)
    m = n - 1;
    inds = (1:m)';
    I = [inds; inds];
    J = [inds; 1+inds];
    S = [ones(m, 1); -ones(m, 1)];
    X = sparse(I, J, S, m, n);
end
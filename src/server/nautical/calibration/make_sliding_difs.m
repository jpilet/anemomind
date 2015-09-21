function difs = make_sliding_difs(n, m)
   inds = (1:n)';
   shift = m - inds(end);
   I = [inds; inds];
   J = [inds; inds + shift];
   X = [ones(size(inds)); -ones(size(inds))];
   difs = sparse(I, J, X, n, m);
end
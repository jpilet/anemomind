function x = minimize_objf(objf, along_dim, Xest)
  Xbut = [Xest; 1];
  Xbut(along_dim) = 0;
  P = objf(:, along_dim);
  Q = objf*Xbut;
  x = -P\Q;
end

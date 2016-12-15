function iterate_game(objf1, objf2, X, g)
  n = 10;
  values = zeros(2, n);
  values(:, 1) = X;
  for i = 2:n,
    weights = (g.^(0:(i-2)))';
    Xest = (values(:, 1:(i-1))*weights)./(sum(weights));
    X = [minimize_objf(objf1, 1, Xest) minimize_objf(objf2, 2, Xest)]';
    Xlast = values(:, i-1);
    Xp = [minimize_objf(objf1, 1, Xlast) minimize_objf(objf2, 2, Xlast)]';
    values(:, i) = X;
    if i == n,
      fprintf('Iteration %d', i);
      fprintf('  Instability: %.3g\n', norm(Xp - Xlast));
      fprintf('  X = \n');
      disp(X);
    end
  end
  %plot(values(1, :), values(2, :));
end

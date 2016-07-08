function Y = make_continuous(X, period)
  half_period = 0.5*period;
  Y = 0*X;
  Y(1) = X(1);
  for i = 2:numel(X),
    dif = X(i) - Y(i-1);
    Y(i) = mod(dif + half_period, period) - half_period + Y(i-1);
  end
end
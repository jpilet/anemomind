function Y = make_continuous(X)
  Y = 0*X;
  Y(1) = X(1);
  for i = 2:numel(X),
    dif = X(i) - Y(i-1);
    Y(i) = adjust(dif) + Y(i-1);
  end
end

function y = adjust(x)
  if x < -180,
    y = x + 360;
  elseif x > 180,
    y = x - 360;
  else
    y = x;
  end
end
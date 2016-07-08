function validate_time(x)
  for i = 2:numel(x),
    y = x(i) - x(1);
    assert(y + 1 == i);
  end
end
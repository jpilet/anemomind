function out = eval_objf(objf, X)
  Y = objf*[X; 1];
  out = dot(Y, Y);
end

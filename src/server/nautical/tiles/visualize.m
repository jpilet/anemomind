data = load('/tmp/tilestep0_dispatcher_matrix.txt');

total_data_size = size(data, 1)

if 1,
  i = 19;
  k = 1000;
  offset = i*1000 + 1;
  inds = (offset+1):(offset + k);
else
  inds = 1:total_data_size;
end

data = data(inds, :);

fprintf("For %.3g seconds\n", data(end, 1) - data(1, 1));

time = data(:, 1);
X = data(:, 2);
Y = data(:, 3);

k = 10000
good = abs(X) < k & abs(Y) < k;

plot(X(good), Y(good), 'b');
axis xy equal

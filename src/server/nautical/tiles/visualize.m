data = load('/tmp/tilestep0_dispatcher_matrix.txt');

time = data(:, 1);
X = data(:, 2);
Y = data(:, 3);

k = 10000
good = abs(X) < k & abs(Y) < k;

plot(X(good), Y(good), 'b');
axis xy equal

data = load('/tmp/tilestep0_dispatcher_matrix.txt');

total_data_size = size(data, 1)

if 0,
  i = 19;
  k = 1000;
  offset = i*1000 + 1;
  inds = (offset+1):(offset + k);
end

if 0,
  inds = 1:total_data_size;
end

if 1,
  inds = abs(data(:, 1)) < 10*60;
end

data = data(inds, :);

fprintf("For %.3g seconds\n", data(end, 1) - data(1, 1));

time = data(:, 1);
X = data(:, 2);
Y = data(:, 3);

twa0 = make_continuous(data(:, 4));
twa1 = make_continuous(data(:, 5));
twa2 = make_continuous(data(:, 6));

if false,
  k = 10000;
  good = abs(X) < k & abs(Y) < k;

  plot(X(good), Y(good), 'b');
  axis xy equal
end

if true,
  plot(time, twa0, 'b');
  hold on
  plot(time, twa1, 'r');
  plot(time, twa2, 'k');
  hold off
  
  legend('Our', 'NMEA2000', 'Ground truth CSV');
end
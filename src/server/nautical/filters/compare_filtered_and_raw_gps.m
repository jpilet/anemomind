index = 7;

prefix = '/tmp';

raw_name = sprintf('%s/raw_positions_%d.txt', prefix, index)
filtered_name = sprintf('%s/filtered_positions_%d.txt', prefix, index)

R = load(raw_name);
F = load(filtered_name);

n = size(F, 1);
step = floor(n/10);

T = 0.001*(F(2:end, 1) - F(1:(end- 1), 1));
dXY = F(2:end, 2:3) - F(1:(end-1), 2:3);
dists = sqrt(sum(dXY.^2, 2));
speeds = dists./T;
fprintf('Max speed meter per seconds: %.3g\n', max(speeds));





show_raw = false;

plot(F(:, 2), F(:, 3), 'g');
for i = 1:step:n,
  text(F(i, 2), F(i, 3), sprintf('At %d', i));
end
if show_raw,
  hold on
  plot(R(:, 2), R(:, 3), 'r');
  hold off
end
axis ij equal
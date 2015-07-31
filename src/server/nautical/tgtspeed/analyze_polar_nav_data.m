data = sortrows(load('/tmp/polarnavdata.txt', '-ascii'), 4);



boat_speed = data(:, 1);
wind_speed = data(:, 2);
wind_angle = data(:, 3);
stability = data(:, 4);

n = size(data, 1);

colors = jet(100);
stability_colors = colors(round(stability*99) + 1, :);

ms = 30
%% Test 1
for i = 1:n,
    plot(wind_speed(i), boat_speed(i), '.', 'MarkerSize', ms, 'Color', stability_colors(i, :));
    hold on
end
hold off

%% Test 2: Split it in two
middle = floor(0.5*n);
first_half = 1:middle;
second_half = middle:n;
plot(wind_speed(first_half), boat_speed(first_half), '.k', 'MarkerSize', ms);
hold on
plot(wind_speed(second_half), boat_speed(second_half), '.r', 'MarkerSize', ms);
hold off

%% Test 3: Top plot
for i = 1:n,
    x = wind_speed(i)*cos(wind_angle(i));
    y = wind_speed(i)*sin(wind_angle(i));
    plot(x, y, '.', 'Color', stability_colors(i, :));
    hold on
end
hold off
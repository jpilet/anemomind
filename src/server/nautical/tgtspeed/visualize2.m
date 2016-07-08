prefix = '/tmp/targetspeeddata.txt';
data = load(strcat(prefix, '_data.txt'));
data(1, :) = data(1, :) - data(1, 1);

short_names = read_lines(strcat(prefix, '_shortnames.txt'));
source_names = read_lines(strcat(prefix, '_sourcenames.txt'));
unit_names = read_lines(strcat(prefix, '_units.txt'));

validate_time(data(1, :));
t = data(1, :);

[data_subset, label_subset] = plot_for_shortname('vmg', 
   short_names, source_names, unit_names, data);
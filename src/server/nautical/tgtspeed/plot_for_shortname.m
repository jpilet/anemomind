function [rows, selected_labels] = plot_for_shortname(short_name, short_names, source_names, unit_names, data)
  t = data(1, :);
  inds = [];
  counter = 0;
  c = 'rgbk';
  for i = 1:numel(short_names),
    if strcmp(short_name, short_names{i}),
      counter = counter + 1;
      inds = [inds i];
      Y = data(i, :);
      if strcmp(unit_names{i}, 'radians') == 1,
        Y = make_continuous(Y, 2.0*pi);
      end
      assert(numel(t) == numel(Y));
      if strcmp(source_names{i}, 'FILTERED'),
         plot(t, Y, 'x', 'Color', c(counter));
      else
         plot(t, Y, 'Color', c(counter));
      end
      hold on
    end
  end
  hold off
  
  title(short_name);
  make_legend(source_names, inds);
  rows = data(inds, :);
  selected_labels = source_names(inds);
end
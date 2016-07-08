function make_legend(labels, inds)
  n = numel(inds);
  Legend=cell(n,1);
  for i = 1:n,
    Legend{i} = labels{inds(i)};
  end
  legend(Legend);
end

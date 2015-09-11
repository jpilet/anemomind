function ds = get_dataset(p)
    full_path = fullfile('/home/jonas/data/datasets', p);
    ds = sortrows(load(full_path, '-ascii'), 1);
    ds(:, 1) = 0.001*ds(:, 1);
end
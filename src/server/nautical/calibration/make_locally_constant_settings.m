function settings = make_locally_constant_settings()
    settings = [];
    settings.good_frac = 0.1;
    settings.iters = 30;
    %settings.weighting_strategy = 'thresholded';
    settings.weighting_strategy = 'best';
end
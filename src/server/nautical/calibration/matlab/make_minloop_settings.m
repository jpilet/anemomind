function settings = make_minloop_settings()
    settings = [];
    settings.visualize = true;
    settings.norm_type = 1; %% <-- 1 for l1, 2 for l2
    settings.chunk_size = 10000;
    settings.overlap = 0.5;
end
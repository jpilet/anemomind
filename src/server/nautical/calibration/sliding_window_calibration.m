function avg_flows = sliding_window_calibration(A, B, coef_count, window_size)
    overlap = 0.1;
    step = floor(overlap*window_size);
    [arows, acols] = size(A);
    [brows, bcols] = size(B);
    assert(arows == brows);
    assert(bcols == 1);
    count = floor(arows/2);
    assert(2*count == arows);
    
    visualize = true;
    
    flows = zeros(count, 2);
    counts = zeros(count, 1);
    for i = 1:step:(count-window_size+1),
        r = i:(i+window_size-1);
        assert(r(end) <= count);
        r2 = range_to_higher_dim(r, 2);
    
        try
            [~, F_, Fs_] = calibrate_locally(A(r2, :), B(r2, :), coef_count);
            F = get_array(Fs_, 2);
            if visualize,
                subplot(1, 2, 1);
                plot(r, F(:, 1));
                hold on
                subplot(1, 2, 2);
                plot(r, F(:, 2));
                hold on
            end

            flows(r, :) = flows(r, :) + F;
            counts(r) = counts(r) + 1;
        catch e,
        end
    end
    avg_flows = scale_rows(1.0e-9 + 1.0./counts, flows);

    r = 1:count;
    if visualize,
        subplot(1, 2, 1);
        plot(r, avg_flows(:, 1), 'g');
        subplot(1, 2, 2);
        plot(r, avg_flows(:, 2), 'g');
        hold off
    end
end
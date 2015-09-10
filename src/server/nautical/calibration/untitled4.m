function flows = sliding_window_calibration(A, B, coef_count, window_size)
    step = floor(0.5*window_size);
    [arows, acols] = size(A);
    [brows, bcols] = size(B);
    assert(arows == brows);
    assert(bcols == 1);
    count = floor(arows/2);
    assert(2*count == arows);
end
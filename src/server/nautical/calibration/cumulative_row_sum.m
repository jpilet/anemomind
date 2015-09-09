function rowsum = cumulative_row_sum(A, step)
    [rows, cols] = size(A);
    rowsum = zeros(rows + step, cols);
    count = floor(rows/step);
    assert(count*step == rows);
    for i = 1:count,
        r0 = get_range(i, step);
        r1 = get_range(i+1, step);
        rowsum(r1, :) = rowsum(r0, :) + A(r0, :);
    end
end
function [blocks, block_count] = calc_avg_blocks(X, block_size, dim)
    [rows, cols] = size(X);
    block_dim = block_size*dim;
    block_count = floor(rows/block_dim);
    avg = (1.0/block_size)*ones(1, block_size);
    avg2 = kron(avg, eye(dim, dim));
    blocks = zeros(block_count*dim, cols);
    for i = 1:block_count,
        blocks(get_range(i, dim), :) = avg2*X(get_range(i, block_dim), :);
    end
end
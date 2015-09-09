function Y = make_smooth(X, dim, order, lambda)
    [rows, cols] = size(X);
    count = floor(rows/dim);
    assert(dim*count == rows);
    fprintf('Making it smooth');
    A = lambda*kron(make_difs_sparse(count, order), speye(dim, dim));
    Y = [speye(rows, rows); A]\[X; zeros(size(A, 1), cols)];
    fprintf('Done!');
end
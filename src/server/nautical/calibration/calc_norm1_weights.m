function out = calc_norm1_weights(residuals, dim, marg)
    if nargin < 3,
        marg = 1.0e-6;
    end
    assert(is_col_vec(residuals));
    R = get_array(residuals, dim);
    e = max(marg, abs(calc_row_norms(R)));
    w0 = 0.5./sqrt(e);
    W = w0(:, ones(1, dim))';
    out = W(:);
end
function F = make_poly_mat(n, coef_count)
    I = (1:n)';
    coefs = 0:(coef_count-1);
    F = I(:, ones(1, coef_count)).^coefs(ones(n, 1), :);
end
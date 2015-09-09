function scaledA = scale_rows(s, A)
    [sr, sc] = size(s);
    [ar, ac] = size(A);
    assert(sr == ar);
    assert(sc == 1);
    scaledA = s(:, ones(1, ac)).*A;
end
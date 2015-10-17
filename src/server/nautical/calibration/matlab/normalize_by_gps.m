function [A, B] = normalize_by_gps(A, B)
    speed = calc_row_norms(get_array(B, 2));
    f = 1.0./(1.0e-9 + speed);
    S = [f f]';
    %A = scale_rows(S(:), A);
    B = scale_rows(S(:), B);
end
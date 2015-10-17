function Q = make_ortho_basis(A)
    [Q, ~] = qr(A);
end
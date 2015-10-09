function indeparam = make_indeparam(Q, B)
    avgQ = apply_avg(Q, 2);
    avgB = apply_avg(B, 2);
    param_count = size(Q, 2);
    outA = zeros(param_count);
    outB = zeros(param_count, 1);
    for i = 1:param_count,
        J = eye(param_count);
        J(i, i) = 0;
        P = -avgQ(:, i);
        outA(i, :) = P\(avgQ*J);
        outB(i, :) = P\avgB;
    end
    indeparam = [];
    indeparam.A = outA;
    indeparam.B = outB;
end
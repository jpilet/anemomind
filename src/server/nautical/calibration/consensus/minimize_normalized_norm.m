function [x, fun, fun2] = minimize_normalized_norm(A, B, C)
% Minimize w.r.t. x:  |A*x + B + C|^2/|A*x + B|^2
    AtA = dot(A, A);
    AtB = dot(A, B);
    AtC = dot(A, C);
    BtB = dot(B, B);
    BtC = dot(B, C);
    CtC = dot(C, C);

    a = AtA;
    b = 2*(AtB + AtC);
    c = 2*BtC + CtC + BtB;
    
    d = AtA;
    e = 2*AtB;
    f = BtB;
    
    [x, fun] = minimize_quad_fraction([a b c], [d e f]);
    fun2 = @verfun;
    
    function out = verfun(X)
        f_num = norm(A*X + B + C)^2
        f_denom = norm(A*X + B)^2
        abc = a*X^2 + b*X + c
        def = d*X^2 + e*X + f
        out = abc/def
    end
end
function [min_value, fun] = minimize_quad_fraction(numerator, denominator)
    assert(numel(numerator) == 3);
    assert(numel(denominator) == 3);
    a = numerator(1);
    b = numerator(2);
    c = numerator(3);
    
    d = denominator(1);
    e = denominator(2);
    f = denominator(3);
    
    A = (a*e - b*d);
    B = 2*(a*f - c*d);
    C = b*f - c*e;
    r = roots([A B C]);
    fun = @(x)(eval_f(numerator, denominator, x));
    if fun(r(1)) < fun(r(2)),
        min_value = r(1);
    else
        min_value = r(2);
    end
end

function y = eval_f(numerator, denominator, x)
    exps = [2 1 0];
    X = [x x x].^exps;
    y = dot(numerator, X)/dot(denominator, X);
end
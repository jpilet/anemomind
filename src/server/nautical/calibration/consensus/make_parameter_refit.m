function refit = make_parameter_refit(Q, B, R)
    refit = [];
    
    refit.A = Q\apply_avg(Q, 2);
    refit.B = Q\(apply_avg(B, 2) - B);
    refit.RAR = (R\refit.A)*R;
    refit.RB = (R\refit.B);
end
function visualize_flow_constancy_errors(E)
    E = E(~isnan(E));
    Esorted = sort(E);
    plot(Esorted);
    
    for p = 0:0.01:1,
        index = round(p*(size(E, 1)-1)) + 1;
        fprintf('Average flow error (knots) at %.3g: %.3g\n', p, Esorted(index));
    end
end
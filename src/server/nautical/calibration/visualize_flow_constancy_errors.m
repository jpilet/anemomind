function visualize_flow_constancy_errors(E)
    colsum = sum(E, 2);
    E = E(~isnan(colsum), :);
    Etrue_flow_sorted = sort(E(:, 1));
    Eapparent_flow_sorted = sort(E(:, 2));
    
    E(:, 1) - E(:, 2)

    fprintf('***** SORTED ERRORS *****\n');
    for p = 0:0.01:1,
        index = round(p*(size(E, 1)-1)) + 1;
        fprintf('Average true error at %.3g:     %.3g\n', p, Etrue_flow_sorted(index));
        fprintf('Average apparent error at %.3g: %.3g\n\n', p, Eapparent_flow_sorted(index));
    end
end
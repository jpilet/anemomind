function [Eflow_sorted, order] = visualize_flow_constancy_errors(E)
    colsum = sum(E, 2);
    valid = ~isnan(colsum);
    E(~valid, :) = inf;
    [Eflow_sorted, order] = sort(E(:, 1));

    n = size(E, 1);
    middle = floor(n/2);
    frac = 0.1;
    first_x = floor(n*frac);
    
    subplot(1, 2, 1);
    plot(Eflow_sorted(1:middle));
    xlabel('Index');
    ylabel('Cross validation error (knots)');

    X = 1:n;
    subset = order(1:first_x);
    
    subplot(1, 2, 2);
    plot(X(valid), E(valid, 1));
    hold on
    plot(X(subset), E(subset, 1), '.r', 'MarkerSize', 12);
    hold off
    xlabel('Index');
    ylabel('Cross validation error');
    
    
    fprintf('***** SORTED ERRORS *****\n');
    for p = 0:0.1:1,
        index = round(p*(size(E, 1)-1)) + 1;
        fprintf('Average error in knots at %.3g (index %d): %.3g\n', p, index, Eflow_sorted(index));
    end
end
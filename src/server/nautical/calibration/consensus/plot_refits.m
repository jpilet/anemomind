function plot_refits(refits, params, range)
    first_refit = refits(1).data;
    if isempty(params),
        params = make_default_params(size(first_refit.RAR, 1));
    end
    if isempty(range),
        range = 1:2;
    end
        
    n = numel(refits);
    X = zeros(n, numel(range));
    for i = 1:n,
        r = refits(i).data;
        x = [i; (r.RAR*params + r.RB)];
        X(i, :) = x(range+1);
    end
    plotx(X, '.k');
    hold on
    plotx(params(range)', '.r', 'MarkerSize', 30);
    hold off
    axis equal;
end
function plot_indeparam_optimizers(optimizers, X)
    Xest = evaluate_indeparam_estimates(optimizers, X);
    bin_count = 300;
    rel_marg = 0.01;
    for i = 1:numel(X),
        subplot(numel(X), 1, i);
        xi = X(i);
        ests = Xest(:, i);
        ests = sort(ests(isfinite(ests)));
        est_count = numel(ests);
        marg = round(rel_marg*est_count);
        ests = ests(marg:(est_count - marg));
        data = hist(ests, bin_count);
        
        hist(ests, bin_count);
        hold on
        plotx([xi 0; xi max(data)], 'r', 'LineWidth', 3);
        hold off
    end
end
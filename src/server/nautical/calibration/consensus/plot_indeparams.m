function plot_indeparams(indeparams, X, index)
    if nargin < 3,
        n = numel(X);
        for i = 1:n,
            subplot(n, 1, i);
            plot_indeparams(indeparams, X, i);
        end
    else
        n = numel(indeparams);
        values = zeros(n, 1);
        bin_count = 30;
        for i = 1:n,
            indeparam = indeparams(i).data
            values(i) = indeparam.A(index, :)*X + indeparam.B(index);
        end
        max_count = max(hist(values, bin_count));

        hist(values, bin_count);
        hold on
        plotx([X(index) 0; X(index) max_count], 'r', 'LineWidth', 3);
        hold off
    end
end
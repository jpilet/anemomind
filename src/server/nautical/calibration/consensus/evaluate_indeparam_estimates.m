function Xest = evaluate_indeparam_estimates(optimizers, X)
    n = numel(optimizers);
    Xest = zeros(n, numel(X));
    for i = 1:n,
        Xest(i, :) = optimizers(i).data(X);
    end
end
function Y = integrate_trajectory(X)
    Y = zeros(size(X));
    for i = 1:size(X, 2),
        Y(:, i) = cumsum(X(:, i));
    end
end
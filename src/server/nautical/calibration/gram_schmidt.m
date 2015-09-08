function Y = gram_schmidt(X)
    n = size(X, 2);
    Y = zeros(size(X));
    Y(:, 1) = normalize_vector(X(:, 1));
    for i = 2:n,
        Ysub = Y(:, 1:(i-1));
        x = X(:, i);
        Y(:, i) = normalize_vector(x - Ysub*(Ysub'*x));
    end
end
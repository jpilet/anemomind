function make_consensus_plot(A_, B)
    A = A_(:, 1:2);
    n = get_observation_count(A);
    count = 30000;
    pts = zeros(count, 2);
    uncertainty = zeros(count, 1);
    max_size = 1000;
    for i = 1:count,
        if mod(i, 100) == 0,
            fprintf('Computing point %d of %d\n', i, count);
        end
        %from_to = sort(randi(n, 1, 2));
        %size = randi(max_size, 1, 1);
        size = max_size;
        from = randi(n - size, 1, 1);
        to = from + size;
        from_to = [from to];
        
        r = range_to_higher_dim(make_range_from_endpoints(from_to), 2);
        [pts(i, :), uncertainty(i)] = fit_params(A(r, :), B(r, :));
    end
    [~, order] = sort(uncertainty);
    plotx(pts(uncertainty < 0.5, :), '.k', 'MarkerSize', 12);
    axis equal;
end

function params = fit_params2(A, B)
    n = get_observation_count(A);
    all_p = [A kron(ones(n, 1), eye(2, 2))]\(-B);
    params = all_p(1:2);
end

function [params, uncertainty] = fit_params(A, B)
    Q = gram_schmidt(A(:, 1:2));
    n = get_observation_count(A);
    R = Q'*A;
    %coeff_count = 4;
    %[K, opt] = make_fitness([-acc(B) kron(make_poly_mat(n+1, coeff_count), eye(2, 2))], acc(Q));
    [K, opt] = make_fitness([-B kron(ones(n, 1), eye(2, 2))], Q);
    try
        KtK = K'*K;
        p = calc_smallest_eigvec(KtK);
        [V, D] = eig(KtK);
        uncertainty = min(D(:))/max(D(:));
    catch e,
        uncertainty = 1;
        return
    end
    scale = opt(1, :)*p;
    params = (1/scale)*(R\p);
    params = normalize_vector(params);
end

function Y = acc(X)
    Y = cumulative_row_sum(X, 2);
end
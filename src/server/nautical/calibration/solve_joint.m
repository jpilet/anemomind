function [wind_params, current_params] = solve_joint(Aw, Bw, Ac, Bc, lambda)
    visualize = true;

    [arows, w_cols] = size(Aw);
    [a2rows, c_cols] = size(Ac);
    assert(arows == a2rows);
    assert(size(Bw, 2) == 1);
    assert(size(Bc, 2) == 1);
    assert(norm(Bw - Bc) < 1.0e-9);
    AB = [Aw zeros(arows, c_cols) Bw; Aw -Ac Bw-Bc; zeros(arows, w_cols) Ac Bc];
    basis = gram_schmidt(AB);
    R = basis'*AB;
    
    wind = make_flow_eqs(basis(get_range(1, arows), :));
    dif = make_flow_eqs(basis(get_range(2, arows), :));
    current = make_flow_eqs(basis(get_range(3, arows), :));
    
    Q = make_Q(wind, dif, current, lambda);
    
    QtQ = Q'*Q;
    [V, D] = eig(QtQ);
    
    if visualize,
        plot(diag(D));
    end
    
    raw_params = calc_smallest_eigvec(QtQ);
    scaled_ab_params = R\raw_params;
    a_params = (1.0/scaled_ab_params(end))*scaled_ab_params(1:(end-1));
    wind_params = a_params(1:w_cols);
    current_params = a_params((w_cols+1):end);
    
    default_a = [make_default(w_cols); make_default(c_cols); 1];
    
    default_params = R*default_a;
    params = R*[a_params; 1];
    
    if visualize,
        figure;
        subplot(1, 3, 1);
        plot_smoothed(wind, params, default_params, lambda);
        subplot(1, 3, 2);
        plot_smoothed(dif, params, default_params, lambda);
        subplot(1, 3, 3);
        plot_smoothed(current, params, default_params, lambda);
    end
end

function Q = make_Q(wind, dif, current, lambda)
    Q = [smoothed(wind, lambda) - wind; smoothed(dif, lambda) - dif; ...
        smoothed(current, lambda) - current];    
end

function Y = make_default(n)
    Y = zeros(n, 1);
    Y(1) = 1.0;
end

function plot_smoothed(basis, X, Xdefault, l)
    plotx(v2(basis*X), 'b');
    hold on
    plotx(v2(make_smooth(basis*X, 2, 2, l)), 'r');
    plotx(v2(basis*Xdefault), 'k');
    hold off
    axis equal
end

function Ac = make_flow_eqs(A)
    Ac = cumulative_row_sum(apply_avg(A, 2), 2);
end

function [avg, V] = calc_avg(A, dim)
    [arows, acols] = size(A);
    n = floor(arows/dim);
    assert(n*dim == arows);
    V = kron(ones(1, n), eye(dim, dim));
    avg = (1.0/n)*V*A;
end

function Ahat = apply_avg(A, dim)
    [avg, V] = calc_avg(A, dim);
    Ahat = A - V'*avg;
end

function Y = nmd(X)
    Y = normalize_vector(X);
end

function Y = v2(X)
    Y = get_array(X, 2);
end

function Y = smoothed(X, l)
    Y = make_smooth(X, 2, 2, l);
end
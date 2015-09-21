function params = calibrate_soap_bubble(A, B, settings)
    if nargin < 3,
        settings = make_soap_bubble_settings();
    end
    curve = make_bubble_curve([A B]);
    Q = gram_schmidt(curve);
    R = Q'*curve;
    n = floor(size(Q, 1)/2);
    edges = kron(make_difs_sparse(n, 2), eye(2))*Q;
    %params0 = minimize_norm1_homogeneous(edges, 2, 30);
    params0 = calc_smallest_eigvec(edges'*edges);
    params1 = R\params0;
    params = (1.0/params1(end))*params1(1:(end-1));
    
    default_params = zeros(size(params));
    default_params(1) = 1;
    dparams0 = normalize_vector(R*[default_params; 1]);
    
    if settings.visualize,
        subplot(1, 2, 1);
        plotx(get_array(Q*params0, 2));
        hold on
        plotx(get_array(Q*dparams0, 2), 'k');
        hold off
        axis equal;
        
        subplot(1, 2, 2);
        plotx(get_array(cumulative_row_sum(A*params + B, 2), 2));
        hold on
        plotx(get_array(cumulative_row_sum(A*default_params + B, 2), 2), 'k');
        hold off
        axis equal
    end
end

function Y = make_bubble_curve(X)
    Y = apply_avg(cumulative_row_sum(apply_avg(X, 2), 2), 2);
end
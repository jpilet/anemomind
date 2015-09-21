function params = calibrate_relative(A, B, settings)
    if nargin < 3,
        settings = make_relative_settings();
    end
    param_count = size(A, 2);
    Ahat = gram_schmidt(A);
    relative_flow = cumulative_row_sum(Ahat, 2);
    gps = cumulative_row_sum(B, 2);
    rows = size(relative_flow, 1);
    count = floor(rows/2);
    difs = kron(make_difs_sparse(count, 2), eye(2));
    difdim = size(difs, 1);
    relative_flow2 = [-gps speye(rows, rows)];
    reg = [zeros(difdim, 1) settings.lambda*difs];
    P = [relative_flow2; reg];
    opt = P\[relative_flow; zeros(difdim, param_count)];
    
    Q = relative_flow2*opt - relative_flow;
    params0 = calc_smallest_eigvec(Q'*Q);
    scale = opt(1, :)*params0;
    flow = (1.0/scale)*opt(2:end, :)*params0;
    
    plotx(get_array(flow, 2)); axis equal;
    
    Ac = cumulative_row_sum(A, 2);
    Bc = cumulative_row_sum(B, 2);
    params = recover_scaled_params(Ac, Bc, flow);
    
    default_params = zeros(size(params));
    default_params(1) = 1;
    
    if settings.visualize,
        plotx(v2(Ac*params + Bc));
        hold on
        plotx(v2(Ac*default_params + Bc), 'k');
        plotx(v2(flow), 'g');
        hold off
        axis equal;
    end
end

function X = v2(arr)
    X = get_array(arr, 2);
end
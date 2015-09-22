function params = calibrate_locally_constant(A, B, R, settings)
    n = get_observation_count(A);
    assert(all([2*n, 1] == size(B)));
    assert(size(R, 2) == 2);
    assert(max(R(:)) <= n);
    assert(1 <= min(R(:)));
    assert(all(R(:, 1) <= R(:, 2)));
    range_count = size(R, 1);
    
    Q = gram_schmidt(A);
    QtA = Q'*A;
    
    apparent_flow_count = sum(R(:, 2) - (R(:, 1) - 1));
    rows = 2*apparent_flow_count;
    
    cols = 1 + 4*range_count;
    elements_per_flow = 4;
    elem_count = elements_per_flow*apparent_flow_count;
    
    I = zeros(1, elem_count);
    J = zeros(1, elem_count);
    X = zeros(1, elem_count);

    rhs = zeros(rows, 4);
    
    row_offset = 0;
    
    residual_ranges = make_array(range_count);
    for i = 1:range_count,
        src_from_to = R(i, :);
        src_range = src_from_to(1):src_from_to(2);
        src_range2 = range_to_higher_dim(src_range, 2);
        flow_count = numel(src_range);
        dst_r = get_range(i, 4*flow_count);
        
        [Ilocal, J(dst_r), X(dst_r)] = make_locally_constant_eqs(B(src_range2, :), i);
        I(dst_r) = Ilocal + row_offset;
        
        next_row_offset = row_offset + 2*flow_count;
        
        residual_range = (row_offset+1):next_row_offset;
        rhs(residual_range, :) = Q(src_range2, :);
        
        residual_ranges(i).data = residual_range;
        
        row_offset = next_row_offset;
    end
    
    
    lhs = sparse(I, J, X, rows, cols);
    opt = lhs\rhs;
    scale_mat = opt(1, :);
    flowdifs = [zeros(2*range_count, 1) kron(speye(range_count, range_count), [speye(2, 2) -speye(2, 2)])];
    K = flowdifs*opt;
    
    Ac = cumulative_row_sum(A, 2);
    Bc = cumulative_row_sum(B, 2);
    initial = Ac*[1 0 0 0]' + Bc;
    
    visualize = true;
    imshow(K, []);
    weights = (1/range_count)*ones(range_count, 1);
    for i = 1:settings.iters,
        fprintf('Iteration %d of %d', i, settings.iters);
        wK = scale_rows(kron(weights, [1 1]'), K);
        params0 = calc_smallest_eigvec(wK'*wK);
        raw_residuals = calc_row_norms(get_array(K*params0, 2));
        residuals = max(raw_residuals, get_frac_smallest_residual(raw_residuals, settings.good_frac));
        weights = distribute_weights(residuals);
        
        if visualize,
            middle = floor(numel(residuals)/2);
            sorted_raw = sort(raw_residuals);
            sorted = sort(residuals);
            
            params = recover_params(params0);
            
            subplot(1, 2, 1);
            plot(sorted_raw(1:middle));
            hold on
            plot(sorted(1:middle), 'r');
            hold off
            drawnow;
            subplot(1, 2, 2);
            plotx(v2(initial), 'k');
            hold on
            plotx(v2(Ac*params + Bc));
            hold off
            axis equal
            
            fprintf('Median flow error: %.3g knots', sorted_raw(middle));
            fprintf('Recovered parameters:\n');
            disp(params);
            pause(1);
        end
    end
    params = recover_params(params0);
    
    function p1 = recover_params(p0)
         lscale = scale_mat*p0;
         p1 = (1/lscale)*(QtA\p0);
    end
end

function Y = v2(X)
    Y = get_array(X, 2);
end

function [r, sorted_residuals] = get_frac_smallest_residual(residuals, frac)
    sorted_residuals = sort(residuals);
    numel(residuals)
    n = ceil(frac*numel(residuals));
    r = sorted_residuals(n);
end

function [I, J, X] = make_locally_constant_eqs(B, i)
    flow_count = get_observation_count(B);
    flow_dim = 2*flow_count;
    elem_count = 2*flow_dim;
    I = zeros(elem_count, 1);
    J = zeros(elem_count, 1);
    X = ones(elem_count, 1);
    
    col_offset = 4*(i-1) + 1;
    
    end_points = round(make_linear_table(5, 0, flow_count));
    
    split_A = [range_0_based_end_points(end_points(1), end_points(2)) ...
        range_0_based_end_points(end_points(3), end_points(4))];
    split_B = [range_0_based_end_points(end_points(2), end_points(3)) ...
        range_0_based_end_points(end_points(4), end_points(5))];
    
    rows = 1:(2*flow_count);
    
    % The constant part
    r1 = get_range(1, flow_dim);
    I(r1) = rows;
    J(r1) = 1;
    X(r1) = -B;
    
    % The variable part
    r2 = get_range(2, flow_dim);
    split_A2 = range_to_higher_dim(split_A, 2);
    I(r2(split_A2)) = rows(split_A2);
    J(r2(split_A2)) = col_offset + ones_and_twos(numel(split_A));
    
    % The second split
    split_B2 = range_to_higher_dim(split_B, 2);
    I(r2(split_B2)) = rows(split_B2);
    J(r2(split_B2)) = col_offset + 2 + ones_and_twos(numel(split_B));
end

function Y = ones_and_twos(n)
    Y = repmat([1 2]', n, 1);
end

function r = range_0_based_end_points(a, b)
    r = (a+1):b;
end
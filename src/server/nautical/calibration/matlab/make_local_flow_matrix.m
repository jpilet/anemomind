function out = make_local_flow_matrix(A, B, ranges)
    Q = gram_schmidt(A);
    
    R = Q'*A;
    
    range_count = size(ranges, 1);
    flow_count = sum(ranges(:, 2) - (ranges(:, 1) - 1));
    flow_dim = 2*flow_count;
    element_count = 2*flow_dim;
    I = zeros(element_count, 1);
    J = zeros(element_count, 1);
    X = zeros(element_count, 1);
    row_count = flow_dim;
    col_count = 1 + 2*range_count;
    rhs = zeros(row_count, size(Q, 2));
    
    offset = 0;
    out_ranges = make_array(range_count);
    for i = 1:range_count,
        src_range = make_range_from_endpoints(ranges(i, :));
        n = numel(src_range);
        src_range2 = range_to_higher_dim(src_range, 2);
        
        [cst_range, var_range] = get_dst_ranges(offset, n);
        rows = get_dst_row_range(offset, n);
        cols = get_dst_var_cols(i, n);
        
        out_ranges(i).data = rows;
        
        I(cst_range) = rows;
        I(var_range) = rows;
        J(cst_range) = 1;
        J(var_range) = cols;
        X(cst_range) = -B(src_range2);
        X(var_range) = 1;
        
        rhs(rows, :) = Q(src_range2, :);
        
        offset = offset + numel(src_range);
    end
    lhs = sparse(I, J, X, row_count, col_count);
    
    opt = lhs\rhs;
    K = lhs*opt - rhs;
    
    out = [];
    out.K = K;
    out.opt_scale_and_flows = opt;
    out.out_ranges = out_ranges;
    out.opt_scale = opt(1, :);
    out.opt_flows = opt(2:end, :);
    out.R = R; % R*params1 = params0, where params1 are real and param0 local.
end

function [gps, true_flow] = get_dst_ranges(offset, n)
    index_offset = 4*offset;
    tmp = (1:(2*n));
    gps = index_offset + tmp;
    true_flow = gps(end) + tmp;
end

function rows = get_dst_row_range(offset, n)
    rows = 2*offset + (1:(2*n));
end

function cols = get_dst_var_cols(i, n)
    cols = 1 + 2*(i-1) + kron(ones(n, 1), [1 2]');
end
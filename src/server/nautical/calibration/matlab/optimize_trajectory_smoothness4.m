function params = optimize_trajectory_smoothness4(A, B, block_size, visualize)
    order = 2;
    lambda = 160000;
    
    basis = gram_schmidt([A B]);
    rowsum = cumulative_row_sum(basis, 2);
    smooth = make_smooth(rowsum, 2, order, lambda);
    
    X = optimize_norms(rowsum, smooth);
    
    [params, scale] = recover_scaled_params(A, B, basis*X);
    dc = zeros(size(A, 2), 1);
    dc(1) = 1;
    default_flow = (A*dc + B);
    default_coefs = (basis\default_flow);
    sX = (1/scale)*X;
    if visualize,
        params
        plotx(get_array(rowsum*sX, 2), 'b');
        hold on
        plotx(get_array(smooth*sX, 2), 'r');
        plotx(get_array(rowsum*default_coefs, 2), 'k');
        hold off
    end
end

function X = optimize_norms(A, Asmooth, X)
    assert(all(size(A) == size(Asmooth)));
    [arows, acols] = size(A);
    if nargin < 3,
        Q = A - Asmooth;
        X = calc_smallest_eigvec(Q'*Q);
    end
    n = floor(arows/2);
    assert(2*n == arows);
    D = kron(make_difs_sparse(n, 1), speye(2, 2));
    dA = D*A;
    dB = D*Asmooth;
    iters = 30;
    for i = 1:iters,
        dA0 = reweigh_norm1(dA, X);
        dB0 = reweigh_norm1(dB, X);
        Q = dA0 - dB0;
        X = calc_smallest_eigvec(Q'*Q);
    end
end


function B = reweigh_norm1(A, X)
   inv_lengths = 1.0./(1.0e-6 + calc_row_norms(get_array(A*X, 2))); 
   L = [inv_lengths inv_lengths]';
   B = scale_rows(L(:), A);
end

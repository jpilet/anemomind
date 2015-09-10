function [A_params, flows, filtered_flows] = calibrate_locally(A, B, poly_coef_count)
    [arows, acols] = size(A);
    [brows, bcols] = size(B);
    assert(arows == brows);
    assert(bcols == 1);
    assert(mod(arows, 2) == 0);
    count = arows/2;
    AB = [A B];
    basis = gram_schmidt(AB);
    R = basis'*AB;
    
    T = cumulative_row_sum(basis, 2);
    F = kron(make_poly_mat(count+1, poly_coef_count), eye(2));
    FFT = F*(F\T);
    Q = FFT - T;
    QtQ = Q'*Q;
    raw_params = calc_smallest_eigvec(QtQ);
    AB_params_scaled = R\raw_params;
    AB_params = (1/AB_params_scaled(end))*AB_params_scaled;
    ortho_params = R*AB_params;
    A_params = AB_params(1:(end-1));
    flows = A*A_params + B;
    filtered_traj = FFT*ortho_params;
    filtered_flows = filtered_traj(3:end) - filtered_traj(1:(end-2));
end
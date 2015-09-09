function [F, J, traj, smooth_traj] = smooth_trajectory_objf(A, B, lambda, block_size, abk)
    [arows, acols] = size(A);
    [brows, bcols] = size(B);
    assert(arows == brows);
    assert(acols == 4);
    assert(bcols == 1);
    assert(isscalar(lambda));
    
    with_k = numel(abk) == 3;
    assert(all(size(abk) == [3 1]) || all(size(abk) == [2 1]));
    if ~with_k,
        abk = [abk; 0];
    end
    
    
    block_count = floor(arows/block_size);
    block_dim = block_count*block_size;
    [abk2, Jabk2] = map_abk(abk);
    
    flows = A*abk2 + B;
    Jflows = A*Jabk2;
    
    hat = zeros(block_dim, 1);
    Jhat = zeros(block_dim, 3);
    for i = 1:block_count,
        r = get_range(i, block_size);
        [hat(r, :), Jhat(r, :)] = normalize_with_J_compact(flows(r, :), Jflows(r, :));
    end
    
    traj = cumulative_row_sum(hat, 2);
    Jtraj = cumulative_row_sum(Jhat, 2);
    
    S = make_smooth(traj, 2, 2, lambda);
    JS = make_smooth(Jtraj, 2, 2, lambda);
    F = S - traj;
    J = JS - Jtraj;
    
    if ~with_k,
        J = J(:, 1:2);
    end
    
    smooth_traj = S;
end
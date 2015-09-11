function [F, J, traj, smooth_traj] = smooth_trajectory_objf_angled(A, B, lambda, block_size, angle)
    if isscalar(angle),
        abk = [cos(angle) sin(angle) 0]';
        Jabk = [-sin(angle) cos(angle) 0]';
        [F, J0, traj, smooth_traj] = smooth_trajectory_objf(A, B, lambda, block_size, abk);
        J = J0*Jabk;
    else 
        [F, J, traj, smooth_traj] = smooth_trajectory_objf(A, B, lambda, block_size, angle);
    end
end
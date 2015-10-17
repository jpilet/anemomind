function [params, traj, traj_S] = optimize_abk(A, B, block_size, lambda)
    init = [1 0 0]';
    counter = 0;
    settings = make_default_levmar_settings;
    settings.maxiter = 30;
    settings.drawfun = @visualize;
    objf = @(X)(smooth_trajectory_objf(A, B, lambda, block_size, init));
    
    [~, ~, traj_ref] = objf(init);
    params = run_levmar(objf, init, settings);
    [F, J, traj, traj_S] = objf(params);
    fprintf('DONE optimizing\n');
    
    function visualize(abk)
        counter = counter + 1;
        fprintf('Iteration %d\n', counter);
        abk
        [~, ~, traj, traj_S] = objf(abk);
        plotx(get_array(traj, 2), 'b');
        hold on
        plotx(get_array(traj_S, 2), 'g');
        plotx(get_array(traj_ref, 2), 'k');
        hold off
        axis equal;
        drawnow;
    end
end
function disp_for_parameters(A, B, parameters)
    XY = A*parameters(:) + B;
    n = get_observation_count(A);
    r = 1:n;
    X = XY(2*r - 1);
    Y = XY(2*r);
    
    figure;
    plot(X, 'g');
    hold on
    plot(Y, 'r');
    hold off
    
    figure;
    plot(X, Y);
    axis equal
end
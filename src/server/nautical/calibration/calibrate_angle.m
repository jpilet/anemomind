function angles = calibrate_angle(A2, B, ranges)
    visualize = true;
    n = size(ranges, 1);
    colors = jet(n);
    quad_forms = make_array(n);
    angles = zeros(n, 1);
    for i = 1:n,
        r = range_to_higher_dim(make_range_from_endpoints(ranges(i, :)), 2);
        Q = normalize_by_eig(make_angle_quadform(A2(r, :), B(r, :)));
        quad_forms(i).data = Q;
        angles(i) = quadform_angle(Q);
        
        if visualize,
            [X, Y] = make_eig2d_plot(Q);
            plot(X, Y, 'Color', colors(i, :));
            hold on
            drawnow;
        end
    end
    hold off
    figure;
    hist(angles, 30);
end
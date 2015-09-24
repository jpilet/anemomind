function [X, Y] = make_eig2d_plot(Q, n) 
    if nargin < 2,
        n = 30;
    end
    assert(all([2, 2] == size(Q)));
    [V, D] = eig(Q);
    d = diag(D);
    assert(issorted(d));
    maxvec = V(:, 2);
    angle = atan2(maxvec(2), maxvec(1));
    X = make_linear_table(n, -pi, pi);
    mean_y = mean(d);
    amp = max(d) - mean_y;
    Y = mean_y + amp*cos(X - angle);
end
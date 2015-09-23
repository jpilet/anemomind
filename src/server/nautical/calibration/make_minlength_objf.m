function [objf, Xinit, drawf, map_params] = make_minlength_objf(A, B)

    A = apply_avg(A, 2);
    B = apply_avg(B, 2);

    Ac = cumulative_row_sum(A, 2);
    Bc = cumulative_row_sum(B, 2);
    counter = 1;

    mu = 0.0001;
    Xinit = [1 0 0 0 1]';
    objf = @f;
    drawf = @drawfun;
    map_params = @map_opt_params;

    function [F, J] = f(X)
        sX = X(1:(end-1), 1);
        s = X(end);
        [af, afJ] = normalize_by_total_length(A, sX, 2, mu);
        
        fprintf('Normalized length: %.3g\n', sum(calc_row_norms(v2(af))));
        
        tf = af + s*B;
        tf2 = get_array(tf, 2);
        tfJ = [afJ B];
        
        n = size(tf2, 1);
        [F, J2] = calc_row_norms_with_jacobian([tf2 mu*ones(n, 1)]);
        
        fprintf('Total length of true flow: %.3g\n', sum(F));
        
        J = J2(:, logical(kron(ones(1, n), [1 1 0])))*tfJ;
    end

    function drawfun(Xh)
        Xh
        X = map_opt_params(Xh)
        plotx(v2(Ac*X + Bc));
        hold on
        plotx(v2(Ac*[1 0 0 0]' + Bc), 'k');
        hold off
        drawnow;
        title(sprintf('Iteration %d\n', counter), 'FontSize', 12);
        counter = counter + 1;
        pause(1.0);
    end
end

function Y = v2(X)
    Y = get_array(X, 2);
end

function X = map_opt_params(Xh)
    X = (1/Xh(end))*Xh(1:(end-1));
end
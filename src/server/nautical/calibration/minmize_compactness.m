function params = minmize_compactness(A, B)
    homo = true;
    dim = 2;
    iters = 300;
    if homo,
        basis = gram_schmidt([A B]);
    else 
        basis = [A B];
    end
    [m, n] = size(basis);
    X = normalize_vector(ones(n, 1));
    count = m/dim
    center_count = floor(count/1000)
    centers = initialize_centers(get_array(basis*X, 2), center_count)
    I = (1:count)';
    last = [];
    B2 = [basis; basis];
    for i = 1:iters;
        fprintf('ITERATION %d/%d', i, iters);
        Y = get_array(basis*X, 2);
        inds = calc_closest_centers(Y, centers);
        I2 = [I; I]
        J2 = [inds(1).data; inds(2).data];
        E = ones(numel(I2), 1);
        C = sparse(I2, J2, E, numel(E), center_count);
        fprintf('SUM(C) = ');
        full(sum(C))
        C2 = kron(C, speye(2));
        optC = (C2\B2);
        F = C2*optC - B2;
        
        if homo,
            X = calc_smallest_eigvec(F'*F);
        else
            X = [-F(:, 1:(end-1))\F(:, end); 1];
        end
        
        if ~isempty(last),
            difs = sum(last ~= J2);
            fprintf('Reassigned in %d positions', difs);
            if all(last == J2),
                fprintf('STOP');
                break;
            end
        end
        centers = get_array(optC*X, 2);
        last = J2;
    end
    params = recover_scaled_params(A, B, basis*X);
end

function C = initialize_centers(all_centers, n)
    C = all_centers(randperm(size(all_centers, 1)), :);
    C = C(1:n, :);
end

function out = calc_closest_centers(A, B)
    n = 2;
    out = make_array(n);
    assert(size(A, 2) == size(B, 2));
    X2 = calc_squared_difs(A(:, 1), B(:, 1));
    Y2 = calc_squared_difs(A(:, 2), B(:, 2));
    dists = sqrt(X2 + Y2);
    
    for i = 1:n,
        [~, inds] = min(dists');
        out(i).data = inds(:);
        dists(:, inds) = inf;
    end
end

function D2 = calc_squared_difs(A, B)
    A = A(:);
    B = B(:);
    a = numel(A);
    b = numel(B);
    A = A(:, ones(b, 1));
    B = B(:, ones(a, 1))';
    assert(all(size(A) == size(B)));
    D = A - B;
    D2 = D.^2;
end
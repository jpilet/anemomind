function Qhat = normalize_by_eig(Q, weight)
    % Normalize by greatest (weight = 1) eigenvalue,
    % smallest (weight = 0) or any linear combination of
    % greatest and smallest.
    if nargin < 2,
        weight = 1;
    end
    try
        [~, D] = eig(Q);
        d = diag(D);
        k = weight*max(d) + (1 - weight)*min(d);
        Qhat = (1/k)*Q;
    catch e,
        Qhat = zeros(size(Q));
    end
    
end
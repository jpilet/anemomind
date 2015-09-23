function X = sparsity_constrained(A, B, ranges, settings)
    if nargin < 4,
        settings = make_sparsity_constrained_settings();
    end
end
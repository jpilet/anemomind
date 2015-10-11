function plot_indeparam_scales(indeparams, X)
    bin_count = 60;
    n = numel(indeparams);
    scales = zeros(1, n);
    for i = 1:n,
        p = indeparams(i).data;
        scales(i) = 1.0/(p.inv_scale*X);
    end
    scale_max = 100;
    scales = scales(-scale_max < scales & scales < scale_max);
    scales = sort(scales);
    marg = round(0.05*numel(scales));
    hist(scales(marg:(end-marg)), bin_count);
    fprintf('Min scale: %.3g\n', min(scales));
    fprintf('Median scale: %.3g\n', median(scales));
end
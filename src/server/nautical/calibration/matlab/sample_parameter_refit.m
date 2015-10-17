function refit = sample_parameter_refit(Q, B, R, n, minl, maxl, refit_count)
    ranges = sample_ranges(n, minl, maxl, refit_count);
    rows1 = count_range_inds(ranges);
    rows2 = 2*rows1;
    
    Qr = zeros(rows2, size(Q, 2));
    avgQr = zeros(rows2, size(Q, 2));
    avgBr = zeros(rows2, 1);
    Br = zeros(rows2, 1);
    from = 1;
    for i = 1:numel(ranges),
        r2 = range_to_higher_dim(ranges(i).data, 2);
        to = from + numel(r2)-1;
        rows = from:to;
        Qsub = Q(r2, :);
        Bsub = B(r2, :);
        Qr(rows, :) = Qsub;
        Br(rows, :) = Bsub;
        avgQr(rows, :) = apply_avg(Qsub, 2);
        avgBr(rows, :) = apply_avg(Bsub, 2);
    end
    
    
    refit = [];
    
    refit.A = Qr\avgQr;
    refit.B = Qr\(avgBr - Br);
    refit.RAR = (R\refit.A)*R;
    refit.RB = (R\refit.B);
end

function n = count_range_inds(ranges)
    n = 0;
    for i = 1:numel(ranges),
        n = n + numel(ranges(i).data);
    end
end
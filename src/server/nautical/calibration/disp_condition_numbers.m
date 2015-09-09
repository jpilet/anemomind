function disp_condition_numbers(rowsum)
    count = 30;
    from = 1;
    to = 300000;
    lambda = make_linear_table(count, log(from), log(to));
    conds = zeros(size(lambda));
    for i = 1:count,
        fprintf('Condition number %d/%d\n', i, count);
        Q = make_smooth(rowsum, 2, 2, lambda(i)) - rowsum;
        conds(i) = cond(Q);
    end
    plot(lambda, conds);
end
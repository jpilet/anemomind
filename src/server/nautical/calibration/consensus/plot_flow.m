function plot_flow(A, B, X)
    cA = cumulative_row_sum(A, 2);
    cB = cumulative_row_sum(B, 2);
    flow = cA*X + cB;
    plotx(get_array(flow, 2));
end
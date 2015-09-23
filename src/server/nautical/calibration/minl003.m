add_calib_paths;

raw = true;
[A, B] = get_calib_ds(0, raw);

%%

make_consensus_plot(A, B);


%%
Ac = cumulative_row_sum(A, 2);
Bc = cumulative_row_sum(B, 2);
plotx(get_array(Ac*[1 0 0 0]' + Bc, 2));
axis equal;
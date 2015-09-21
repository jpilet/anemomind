add_calib_paths;

index = 0;

raw = true;
[A, B] = get_calib_ds(index, raw);

%%

params = calibrate_straightest(A, B)

%%
relative_flow = cumulative_row_sum(A*[1 0 0 0]', 2);
gps = cumulative_row_sum(B, 2);
absolute_flow = cumulative_row_sum(B + A*[1 0 0 0]', 2);

subplot(1, 4, 1);
plotx(get_array(relative_flow, 2)); axis equal;
subplot(1, 4, 2);
plotx(get_array(gps, 2)); axis equal;
subplot(1, 4, 3);
plotx(get_array(absolute_flow, 2)); axis equal;
subplot(1, 4, 4);
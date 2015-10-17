add_calib_paths;

raw = true;
[A, B] = get_calib_ds(1, raw);
n = get_observation_count(A);
%%
params = calibrate_minlength(A, B)

Ac = cumulative_row_sum(A, 2);
Bc = cumulative_row_sum(B, 2);

plotx(get_array(Ac*params + Bc, 2));
hold on
plotx(get_array(Ac*[1 0 0 0]' + Bc, 2), 'k');
hold off

%%

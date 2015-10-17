add_calib_paths;

ds = get_dataset('exocet_matlab.txt');

%%
from_to_dur = get_time_difs(ds);

r = 1:300;
plot(r, log(from_to_dur(r, 3)));
%%
[ranges, durs] = get_ranges(ds, 60)

r = ranges(11, :)

ds_sub = ds(r(1):r(2), :);
gps_motions = get_gps_motion(ds_sub);
fprintf('Mean GPS speed: %.3g\n', mean(calc_row_norms(gps_motions)));
gps_traj = cumulative_row_sum(gps_motions, 1);
plotx(gps_traj);

for i = 1:size(ranges, 1),
    range = ranges(i, :)
    summary = summarize_dataset(ds(range, :))
    if i <= numel(durs),
        fprintf('%.3g seconds to next', durs(i));
    end
end

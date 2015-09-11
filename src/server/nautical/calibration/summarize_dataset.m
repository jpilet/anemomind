function summary = summarize_dataset(ds)
    gps_speed = calc_row_norms(get_gps_motion(ds));

    summary = [];
    summary.from_time = ds(1, :);
    summary.to_time = ds(end, :);
    summary.max_gps_speed = max(gps_speed);
    summary.min_gps_speed = min(gps_speed);
    summary.median_gps_speed = median(gps_speed);
end
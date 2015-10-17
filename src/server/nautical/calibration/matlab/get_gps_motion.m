function motion = get_gps_motion(ds)
    motion = make_horizontal_motion(deg2rad(ds(:, 12)), ds(:, 11));
end
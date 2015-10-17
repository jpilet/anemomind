add_calib_paths;

index = 4;

raw = true;
[A, B] = get_calib_ds(index, raw);

%%

params = calibrate_minloop(A, B)
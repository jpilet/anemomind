add_calib_paths;

index = 2;

raw = true;
[A, B] = get_calib_ds(index, raw);

%%
format long g
params = calibrate_relative(A, B);

add_calib_paths;

index = 0;

raw = true;
[A, B] = get_calib_ds(index, raw);

%%
params = calibrate_soap_bubble2(A, B)
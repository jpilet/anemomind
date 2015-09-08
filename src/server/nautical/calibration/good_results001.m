% 0: Wind1
% 1: Current1
% 2: Wind2
% 3: Current2
% 4: Wind(synth)
% 5: Current(synth)

params_4 = [0.80673 -0.216292 -0.0438555 0.0241282];

add_calib_paths;

raw = false;
[A, B] = get_calib_ds(3, raw);

n = get_observation_count(A);

r = make_range(1, 4000);

params = [1, 0, 0, 0];
%params = [0, 0, 0, 0];
%params = params_4;
disp_for_parameters(A(r, :), B(r, :), params);

%%
P = A\-B
X0 = [0 1];
X1 = [2 3];
X2 = [4 5];
inds = X1;

flip = false;
if flip,
    inds = inds([2 1]);
end


raw = false;
[Aw, Bw] = get_calib_ds(inds(1), raw);
[Ac, Bc] = get_calib_ds(inds(2), raw);

assert(all(size(Aw) == size(Ac)));
assert(all(size(Bw) == size(Bc)));

%%
params = decorr_calib(Aw, Bw, Ac, Bc, 500)

%%
params = [1, 0, 0, 0];

%%
disp_for_parameters(Aw, Bw, params);
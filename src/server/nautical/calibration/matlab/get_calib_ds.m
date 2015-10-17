function [A, B] = get_calib_ds(index, raw)
    raws = '';
    if raw,
        raws = 'raw';
    end
    base = '/home/jonas/data/datasets/linearcalib';
    aname = fullfile(base, sprintf('A%s_%2d.txt', raws, index));
    bname = fullfile(base, sprintf('B%s_%2d.txt', raws, index));
    A = load(aname, '-ascii');
    B = load(bname, '-ascii');
end
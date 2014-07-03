function out = load_string_matrix(filename)
    file = fopen(filename, 'r');
    [rows, cols] = read_matrix_size(file);
    out = cell(rows, cols);
    ivl = 3.0;
    next = cputime + ivl;
    was_verbose = false;
    for i = 1:rows,
        for j = 1:cols,
            out{i, j} = fgetl(file);
        end
        if cputime > next,
            was_verbose = true;
            next = cputime + ivl;
            fprintf('Read matrix row %d/%d (%.3g percents done)\n', i, rows, 100.0*i/rows);
        end
    end
    fclose(file);
    if was_verbose,
        fprintf('Done.\n');
    end
end

function [rows, cols] = read_matrix_size(file)
    rows = str2num(fgetl(file));
    cols = str2num(fgetl(file));
end
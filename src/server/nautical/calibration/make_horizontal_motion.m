function motions = make_horizontal_motion(angles, velocities)
    assert(is_col_vec(angles));
    assert(is_col_vec(velocities));
    motions = scale_rows(velocities(:), [sin(angles(:)) cos(angles(:))]);
end
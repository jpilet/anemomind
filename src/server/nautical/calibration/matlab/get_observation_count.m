function n = get_observation_count(A)
    n = floor(size(A, 1)/2);
    assert(2*n == size(A, 1));
end
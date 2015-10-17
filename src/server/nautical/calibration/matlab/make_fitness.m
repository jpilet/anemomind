function [F, opt] = make_fitness(A, B)
    opt = A\B;
    F = A*opt - B;
end
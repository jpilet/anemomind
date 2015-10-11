function [Ahat, Bhat] = compress_AB(A, B)
    AtA = A'*A;
    [V, D] = eig(AtA);
    Ahat = sqrt(D)*V';
    AtB = A'*B;
    Bhat = (Ahat')\AtB;
end
function [C, phi] = rewrite_sin_cos_sum(cos_factor, sin_factor)
    % Reqwrite an expression on the form cos_factor*cos(x) +
    % sin_factor*sin(x) to the form C*sin(x + phi)
    C = sqrt(cos_factor^2 + sin_factor^2);
    phi = atan2(cos_factor, sin_factor);
end
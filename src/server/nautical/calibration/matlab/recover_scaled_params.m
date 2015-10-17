function [params, scale] = recover_scaled_params(A, B, flow)
    params = [A B]\flow;
    scale = params(end);
    params = (1.0/scale)*params(1:(end-1));
end
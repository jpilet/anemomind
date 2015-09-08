function params = recover_scaled_params(A, B, flow)
    params = [A B]\flow;
    params = (1.0/params(end))*params(1:(end-1));
end
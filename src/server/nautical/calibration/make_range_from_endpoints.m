function range = make_range_from_endpoints(ep0, opt_ep1)
    if nargin == 1,
        a = ep0(1);
        b = ep0(2);
    else
        assert(nargin == 2);
        a = ep0;
        b = opt_ep1;
    end
    range = a:b;
end
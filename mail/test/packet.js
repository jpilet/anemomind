var pkt = require('../packet.js');
var assert = require('assert');

it(
    'isLightPacket',
    function() {
	describe(
	    'Should distinguish between what is a light packet and what is not',
	    function() {
		assert(!pkt.isLightPacket(undefined));
		assert(!pkt.isLightPacket(1));
		assert(pkt.isLightPacket(
		    {
			src: 1,
			dst: 2,
			seqNumber: 3,
		    }
		));
	    }
	);

    }
);

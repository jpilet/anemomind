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

it(
    'isFullPacket',
    function() {
	describe(
	    'Should distinguish between what is a full packet and what is not',
	    function() {
		assert(!pkt.isFullPacket(undefined));
		assert(!pkt.isFullPacket(1));
		assert(!pkt.isFullPacket(
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

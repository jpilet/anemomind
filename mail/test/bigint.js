var assert = require('assert');
var bigint = require('../bigint.js');


it(
    'bigint',
    function() {
	describe(
	    'Should order and count with big integers correctly',
	    function() {
		assert(bigint.isZero('0000000'));
		assert(!bigint.isZero('0000001'));
		assert(!bigint.isZero('1000000'));
		assert(bigint.isZero(bigint.zero()));
		
		var intTester = function(x, is) {
		    assert(isBigInt(x));
		    assert(isBigIntStrict(x));
		    var nextX = (x);
		    assert(x < nextX || isZero(nextX));
		}

		
	    }
	);
    }
);

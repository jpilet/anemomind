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
		    assert(bigint.isBigInt(x));
		    assert(bigint.isBigIntStrict(x));
		    var nextX = bigint.inc(x);
		    assert(x.length == nextX.length);
		    console.log('x = ' + x);
		    console.log('nextX = ' + nextX);
		    assert(x < nextX || bigint.isZero(nextX));
		}

		intTester('abcde99');
		intTester('fffffff');
		intTester('00000000000000000000000000000000000000000000000000');
		intTester('ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff');
		intTester('fffffffffffffffffffaaaaaaaaaaaafffffffffffffffffff');

		var a = bigint.inc(bigint.inc('00c8'));
		var b = bigint.inc('00c9');
		console.log('a = ' + a);
		console.log('b = ' + b);
		assert(a == b);
		
	    }
	);
    }
);

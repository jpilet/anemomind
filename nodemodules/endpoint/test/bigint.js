var assert = require('assert');
var bigint = require('../bigint.js');
var timesrc = require('../timesrc.js');


function implies(a, b) {
    return !a || b;
}

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
		    if (is == undefined) {
			is = true;
		    }
		    
		    assert(implies(is, bigint.isBigInt(x)));
		    assert(bigint.isBigIntStrict(x) == is);
		    if (is) {
			var nextX = bigint.inc(x);
			assert(x.length == nextX.length);
			assert(x < nextX || bigint.isZero(nextX));
		    }
		}

		intTester('abcde99o', false);
		intTester('    abc   ', false);
		intTester('abcde99');
		intTester('fffffff');
		intTester('00000000000000000000000000000000000000000000000000');
		intTester('ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff');
		intTester('fffffffffffffffffffaaaaaaaaaaaafffffffffffffffffff');

		var a = bigint.inc(bigint.inc('00c8'));
		var b = bigint.inc('00c9');
		assert(a == b);
		
	    }
	);

	describe(
	    'Should serialize and deserialize a big integer correctly',
	    function() {
		{
		    var a = 'fffff1234';
		    assert(bigint.deserializeBigInt(bigint.serialize(a), a.length) == a);
		}{
		    var a = 'ffff1234';
		    assert(bigint.deserializeBigInt(bigint.serialize(a), a.length) == a);
		}
	    }
	);
	
	describe(
	    'Should serialize and deserialize arrays of big integers correctly',
	    function() {
		var a = ['abc', 'cba', '123'];
		var b = bigint.serialize(a);
		var c = bigint.deserializeBigInts(b, 3);

		// Directly asserting a == c doesn't seem to work for some reason:
		assert(a.length == c.length);
		assert(a.length == 3);
		for (var i = 0; i < 3; i++) {
		    assert(a[i] == c[i]);
		}
	    }
	);

	describe('Global time', function() {
	    timesrc.get = function() {
		return new Date(119);
	    }

	    var t = bigint.makeFromTime(8);
	    assert(t == '00000077');

	    timesrc.reset();
	});
    }
);

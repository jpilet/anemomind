// Utilities to work with integers represented as
// strings of fixed length, suitable for
// storage in db and transfer.
// Since they are just strings of _fixed width_,
// the lexical ordering of the string representation of numbers
// is equivalent to the ordering of the numbers.


var assert = require('assert');

// Since the integers are just strings
// that are sorted lexically by the characters,
// just make sure that the digit of 9 is less than
// the digit of 10.
assert('9' < 'a');

// 64 bits = 8 bytes = 16 hexadecimal digits
var int64width = 16;

// The width that we believe is sufficient for assigning unique numbers
// in various situations.
var defaultWidth = int64width;

function isBigInt(x) {
    return typeof x == 'string';
}

function isHexDigit(x) {
    if (x.length == 1) {
	return ('0' <= x && x <= '9')
	    || ('a' <= x && x <= 'f');
    }
    return false;
}

function isBigIntStrict(x) {
    if (isBigInt(x)) {
	for (var i = 0; i < x.length; i++) {
	    if (!isHexDigit(x[i])) {
		return false;
	    }
	}
	return true;
    }
    return false;
}

function isZero(x) {
    if (isBigInt(x)) {
	for (var i = 0; i < x.length; i++) {
	    if (x[i] != '0') {
		return false;
	    }
	}
	return true;
    }
    return undefined;
}

// If x is undefined, return the defaultWidth
function withDefaultWidth(x) {
    return x || defaultWidth;
}

// Repeat string s n times.
function repeatString(s, n) {
    return new Array(n + 1).join(s);
}

// A string representing 0.
function zero(width) {
    return repeatString('0', withDefaultWidth(width));
}

// Add zeros so that the number x has width w.
function padWith0(x, w) {
    assert(typeof x == 'string');
    var remain = w - x.length;
    return (remain <= 0? x : zero(remain) + x);
}

// Make an integer based on the current time,
// that is the number of milliseconds since 1970.
function makeFromTime(width) {
    var x = (new Date()).getTime().toString(16);
    return padWith0(x, withDefaultWidth(width));
}


function make(x, w) {
    assert(typeof x == 'number');
    return padWith0(
	x.toString(16),
	withDefaultWidth(w)
    );
}

// Increase a number by 1, padding with 0 if necessary.
// The number should be possible to represent exactly as a 'number'.
function incSub(x) {
    return padWith0((parseInt(x, 16) + 1).toString(16), x.length);
}

// Returns a new integer increased by one. Any length is OK.
function inc(x) {
    assert(isBigInt(x));
    if (x == '') {
	return x;
    } else {
	var w = 8;
	var k = x.length - w;
	var right = (x.length <= w? x : x.slice(k));
	var left = (x.length <= w? '' : x.slice(0, k));
	assert(left.length + right.length == x.length);
	var rightInc = incSub(right);
	if (rightInc.length > right.length) {
	    return inc(left) + rightInc.slice(1);
	} else {
	    return left + rightInc;
	}	
    }
}

// Todo: randomize strings using
//       the system rng, to assign
//       unique names for mailboxes.


module.exports.zero = zero;
module.exports.isZero = isZero;
module.exports.isBigInt = isBigInt;
module.exports.inc = inc;
module.exports.isBigIntStrict = isBigIntStrict;
module.exports.makeFromTime = makeFromTime;
module.exports.make = make;
module.int64width = int64width;

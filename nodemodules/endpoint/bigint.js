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
assert((9).toString(16) < (10).toString(16));

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
    return !!x.match(/^[0-9a-f]+$/);
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
    assert.equal(typeof x, 'string');
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
    assert.equal(typeof x, 'number');
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
	var w = 8; // 8 hex digits => 4 bytes => A 32 bit int that can be accurately represented.
	var k = x.length - w;
	var right = (x.length <= w? x : x.slice(k));
	var left = (x.length <= w? '' : x.slice(0, k));
	assert.equal(left.length + right.length, x.length);
	var rightInc = incSub(right);
	if (rightInc.length > right.length) {
	    return inc(left) + rightInc.slice(1);
	} else {
	    return left + rightInc;
	}	
    }
}

function calcEvenLength(x) {
    assert.equal(typeof x, 'number');
    var even = (x % 2) == 0;
    return (even? x : x + 1);
}

function calcByteCount(x) {
    return calcEvenLength(x)/2;
}

function padToEvenDigits(x) {
    return padWith0(x, calcEvenLength(x.length));
}


function serializeBigIntToBuffer(x, dstBuffer, dstOffset) {
    dstOffset = (dstOffset == undefined? 0 : dstOffset);
    x = padToEvenDigits(x);
    var bytes = x.length/2;
    for (var i = 0; i < bytes; i++) {
	srcOffset = 2*i;
	dstBuffer.writeUInt8(
	    parseInt(x.slice(srcOffset, srcOffset + 2), 16),
	    dstOffset + i
	);
    }

    // Return the point where we can write the next object.
    return dstOffset + bytes;
}

function serializeBigInt(x) {
    var buf = new Buffer(calcByteCount(x.length));
    assert.equal(serializeBigIntToBuffer(x, buf, 0), buf.length);
    return buf;
    
}

function deserializeBigIntFromBuffer(srcBuffer, srcOffset, dstWidth) {
    var byteCount = calcByteCount(dstWidth);
    var bytes = new Array(byteCount);
    for (var i = 0; i < byteCount; i++) {
	bytes[i] = padWith0(srcBuffer.readUInt8(srcOffset + i).toString(16), 2);
    }
    return bytes.join("").slice(2*byteCount == dstWidth? 0 : 1);
}

function deserializeBigInt(srcBuffer, dstWidth) {
    return deserializeBigIntFromBuffer(
	srcBuffer, 0,
	dstWidth || 2*srcBuffer.length
    );
}


function serialize(x) {
    if (typeof x == 'string') {
	return serializeBigInt(x);
    } else { // Supposedly an array of big ints
	if (x.length == 0) {
	    return new Buffer(0);
	} else {

	    // Require that all integers are equally long. Does that make sense?
	    var len = x[0].length;
	    for (var i = 0; i < x.length; i++) {
		assert.equal(x[i].length, len);
		assert(isBigInt(x[i]));
	    }
	    
	    return serializeBigInt(x.join(""));
	}
    }
}

function deserializeBigInt(buf, width) {
    width = width || 2*buf.length;
    return deserializeBigIntFromBuffer(buf, 0, width);
}

function deserializeBigInts(buf, width) {
    width = withDefaultWidth(width);
    var concatWidth = 2*buf.length;
    var count = Math.floor(concatWidth/width);
    var concatResult = deserializeBigInt(buf, concatWidth);
    concatResult = (count*width == concatWidth? concatResult : concatResult.slice(1));
    var result = new Array(count);
    for (var i = 0; i < count; i++) {
	var offset = width*i;
	result[i] = concatResult.slice(offset, offset + width);
    }
    return result;

}

// Compute the difference between to bigints. Hacky O(n) version.
// This is not a big deal, though, because the context where it is used
// is also O(n), so this doesn't change the complexity.
function diff(a, b) {
  assert(a.length == b.length);
  if (a >= b) {
    var counter = 0;
    while (a > b) {
      b = inc(b);
      counter++;
    }
    return counter;
  } else {
    return -diff(b, a);
  }
}

// Todo: randomize strings using
//       the system rng, to assign
//       unique names for endpoints.


module.exports.serialize = serialize;
module.exports.deserializeBigInt = deserializeBigInt;
module.exports.deserializeBigInts = deserializeBigInts;
module.exports.zero = zero;
module.exports.isZero = isZero;
module.exports.isBigInt = isBigInt;
module.exports.inc = inc;
module.exports.isBigIntStrict = isBigIntStrict;
module.exports.makeFromTime = makeFromTime;
module.exports.make = make;
module.exports.int64width = int64width;
module.exports.defaultWidth = defaultWidth;
module.exports.serializeBigIntToBuffer = serializeBigIntToBuffer;
module.exports.deserializeBigIntFromBuffer = deserializeBigIntFromBuffer;
module.exports.calcByteCount = calcByteCount;
module.exports.diff = diff;

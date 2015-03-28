// Coding/decoding of RPC calls.
var assert = require('assert');
var pkt = require('./packet.js');
var bigint = require('./bigint.js');

// An object with functions to wrap or unwrap another object
function Coder(wrap, unwrap) {
    this.wrap = wrap;
    this.unwrap = unwrap;
}

function isCoder(x) {
    if (typeof x == 'object') {
	return (typeof x.wrap == 'function') &&
	    (typeof x.unwrap == 'function');
    }
    return false;
}

// All the wrapping functions needed to
// perform an RPC call.
function Call(args, result) {
    assert(isCoder(args) || args == undefined);
    assert(isCoder(result) || result == undefined);
    this.args = args;
    this.result = result;
}

module.exports.setForeignDiaryNumber = new Call(
    // Args
    new Coder(
	function(obj) {
	    assert(bigint.isBigInt(obj.mailboxName));
	    assert(bigint.isBigInt(obj.diaryNumber));
	    return bigint.serialize(obj.mailboxName + obj.diaryNumber);
	}, function(wrappedObj) {
	    var arr = bigint.deserializeBigInts(wrappedObj, bigint.defaultWidth);
	    return {
		mailboxName: arr[0],
		diaryNumber: arr[1]
	    };
	}
    ),

    // No data to return => no need for a Coder.
    undefined
);

var packetCoder = new Coder(pkt.serialize, pkt.deserialize);

module.exports.getFirstPacketStartingFrom = new Call(
    // Args
    new Coder(
	function(obj) {
	    assert(bigint.isBigInt(obj.mailboxName));
	    assert(bigint.isBigInt(obj.diaryNumber));
	    assert(typeof obj.lightWeight == 'boolean');
	    var buf = new Buffer(bigint.defaultWidth + 1);
	    var a0 = serializeBigIntToBuffer(obj.mailboxName, buf, 0);
	    var a1 = serializeBigIntToBuffer(obj.diaryNumber, buf, a0);
	    assert(a1 + 1 == buf.length);
	    var a2 = buf.writeUInt8(obj.lightWeight? 1 : 0);
	    return buf;
	}, function(buf) {
	    assert(buf.length == 1 + bigint.defaultWidth);
	    var mailboxName = bigint.deserializeBigIntFromBuffer(buf, 0, bigint.defaultWidth);
	    var diaryNumber = bigint.deserializeBigIntFromBuffer(
		buf,
		bigint.defaultWidth/2, bigint.defaultWidth);
	    var lightWeight = buf.readUInt8(bigint.defaultWidth) != 0;
	}
    ),

    // Return value
    packetCoder
);

module.exports.handleIncomingPacket = new Call(
    // Args: packet
    packetCoder,

    // No return value
    undefined
);

module.exports.isAdmissible = new Call(

    // Args: src, dst
    new Coder(
	function(obj) {
	    return bigint.serialize([obj.src, obj.dst]);
	}, function(wrappedObj) {
	    var arr = bigint.deserializeBigInts(wrappedObj, bigint.defaultWidth);
	    return {
		src: arr[0],
		dst: arr[1]
	    };
	}
    ),

    // Return value, a boolean
    new Coder(
	function(p) {
	    assert(typeof p == 'boolean');
	    var buf = new Buffer(1);
	    buf.writeUInt8(p? 1 : 0, 0);
	    return buf;
	}, function(buf) {
	    assert(buf.length == 1);
	    return buf.readUInt8(0) != 0;
	}
    )
);

module.exports.getForeignDiaryNumber = new Call(
    // Args: a mailbox name
    new Coder(
	function(mailboxName) {
	    return bigint.serialize(mailboxName);
	}, function(wrappedName) {
	    return bigint.deserializeBigInt(wrappedName);
	}
    ),

    // Return value: a diary number.
    new Coder(
	function(diaryNumber) {
	    return bigint.serialize(diaryNumber);
	}, function(wrappedDiaryNumber) {
	    return bigint.deserialize(wrappedDiaryNumber);
	}
    )
);

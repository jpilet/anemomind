var assert = require('assert');

var int64Size = 8;

// src and dst.
var mailboxIdSize = int64Size;

// cNumber, seqNumber, diaryNumber are counters.
var counterSize = int64Size;


// A light packet contains information to determine whether or not
// it should be rejected
function isLightPacket(x) {
    if (typeof x == 'object') {
	return x.src != undefined &&
	    x.dst != undefined &&
	    x.seqNumber != undefined;
    }
    return false;
}

// A full packet is packet that can be stored in the database.
function isFullPacket(x) {
    if (isLightPacket(x)) {
	return x.cNumber != undefined &&
	    x.label != undefined &&
	    x.data != undefined;
    }
    return false;
}

// A packet is assigned a diarynumber when it is put in the database,
// by the host.
function hasDiaryNumber(x) {
    if (isLightPacket(x)) {
	return x.diaryNumber != undefined;
    }
    return undefined; // <-- is this a reasonable return value in case we provide the wrong type?
}

// Helper object to read/write buffers.
function BufferManager(buf) {
    this.pointer = 0;
    this.buffer = buf;
}

// A method that returns whether or not we have reached the end of
// the buffer.
BufferManager.prototype.finished = function() {
    return this.pointer == this.buffer.length;
}

BufferManager.prototype.writeBigInt = function(x, width) {
    assert(!this.finished());
    this.pointer = bigint.serializeBigIntToBuffer(x, this.buffer, this.pointer);
}

BufferManager.prototype.readBigInt = function(width) {
    assert(!this.finished());
    var result = bigint.deserializeBigIntFromBuffer(this.buffer, width);
    this.pointer += bigint.calcByteCount(width);
    return result;
}

BufferManager.prototype.writeUInt8 = function(x) {
    assert(!this.finished());
    this.buffer.writeUInt8(x, this.pointer);
    this.pointer++;
}

BufferManager.prototype.readUInt8 = function() {
    var result = this.readUInt8(this.pointer);
    this.pointer++;
    return result;
}

BufferManager.prototype.writeBuffer = function(buf) {
    buf.copy(this.buffer, this.pointer);
    this.pointer += buf.length;
}

BufferManager.prototype.getRemainingBuffer = function() {
    var result = this.buffer.slice(this.pointer);
    this.pointer = this.buffer.length;
    return result;
}


///// serializeLight/deserializeLight are used when we test if a packet should
// be transferred.
//
// If it is the case that the packet should be transferred, the
// diaryNumber is used as a starting point for querying the next
// packet.
function serializeLight(packet) {
    assert(isLightPacket(packet));
    return bigint.serialize(
	[packet.diaryNumber, packet.src, packet.dst, packet.seqNumber]
    );
}

function isSerializedLightPacket(x) {
    return bigint.calcByteCount(4*bigint.defaultWidth) == x.length;
}

function deserializeLightPacket(x) {
    var arr = bigint.deserializeBigInts(x, bigint.defaultWidth);
    assert(arr.length == 4);
    return {
	diaryNumber: arr[0],
	src: arr[1],
	dst: arr[2],
	seqNumber: arr[3]
    };
}

function serializeFullPacket(packet) {
    var dstLen = calcByteCount(5*bigint.defaultWidth) + 1 + packet.data.length;
}

function deserializePacket(x) {
    var b = new BufferManager(x);
    var diaryNumber = b.readInt(counterSize);
    var src = b.readInt(mailboxIdSize);
    var dst = b.readInt(mailboxIdSize);
    var seqNumber = b.readInt(counterSize);
    var result = {
	diaryNumber: diaryNumber,
	src: src,
	dst: dst,
	seqNumber: seqNumber
    };
    assert(isLightPacket(result));
    return result;
}



// TO BE COMPLETED ONCE WE KNOW THE EXACT PACKET FORMAT
//
///// For full packets
// function serializeFull(packet) {
//     assert(isLightPacket(packet));
    
//      var dst = new BufferManager(
// 	3*counterSize + 2*mailboxIdSize
//     );

//     // To serialize: diaryNumber, src, dst, seqNumber       
//     dst.writeInt(src.diaryNumber, counterSize);
//     dst.writeInt(src.src, mailboxIdSize);
//     dst.writeInt(src.dst, mailboxIdSize);
//     dst.writeInt(src.seqNumber, counterSize);
//     dst.writeInt();

//     assert(dst.finished());
//     return dst;
// }

// function deserializePacket(x) {
//     var b = new BufferManager(x);
//     var diaryNumber = b.readInt(counterSize);
//     var src = b.readInt(mailboxIdSize);
//     var dst = b.readInt(mailboxIdSize);
//     var seqNumber = b.readInt(counterSize);
//     var result = {
// 	diaryNumber: diaryNumber,
// 	src: src,
// 	dst: dst,
// 	seqNumber: seqNumber
//     };
//     assert(isLightPacket(result));
//     return result;
// }

exports.isLightPacket = isLightPacket;
exports.isFullPacket = isFullPacket;
exports.hasDiaryNumber = hasDiaryNumber;

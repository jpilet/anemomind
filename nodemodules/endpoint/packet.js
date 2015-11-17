var assert = require('assert');
var bigint = require('./bigint.js');

var int64Size = 8;

// src and dst.
var endpointIdSize = int64Size;

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
    width = width || bigint.defaultWidth;
    assert(!this.finished());
    this.pointer = bigint.serializeBigIntToBuffer(x, this.buffer, this.pointer);
}

BufferManager.prototype.readBigInt = function(width) {
    width = width || bigint.defaultWidth;    
    assert(!this.finished());
    var result = bigint.deserializeBigIntFromBuffer(this.buffer, this.pointer, width);
    this.pointer += bigint.calcByteCount(width);
    return result;
}

BufferManager.prototype.writeUInt8 = function(x) {
    assert(!this.finished());
    this.buffer.writeUInt8(x, this.pointer);
    this.pointer++;
}

BufferManager.prototype.readUInt8 = function() {
    var result = this.buffer.readUInt8(this.pointer);
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
function serializeLightPacket(packet) {
    if (packet == undefined) {
	return new Buffer(0);
    } else {
	assert(isLightPacket(packet));
	return bigint.serialize(
	    [packet.diaryNumber, packet.src, packet.dst, packet.seqNumber]
	);	
    }
}

function isSerializedLightPacket(x) {
    return bigint.calcByteCount(4*bigint.defaultWidth) == x.length;
}

function deserializeLightPacket(x) {
    if (x.length == 0) {
	return undefined;
    } else {
	var arr = bigint.deserializeBigInts(x, bigint.defaultWidth);
	assert.equal(arr.length, 4);
	return {
	    diaryNumber: arr[0],
	    src: arr[1],
	    dst: arr[2],
	    seqNumber: arr[3]
	};	
    }
}

function serializeFullPacket(packet) {
    assert(isFullPacket(packet));
    var dstLen = bigint.calcByteCount(5*bigint.defaultWidth) + 1 + packet.data.length;
    var b = new BufferManager(new Buffer(dstLen));
    b.writeBigInt(packet.diaryNumber);
    b.writeBigInt(packet.src);
    b.writeBigInt(packet.dst);
    b.writeBigInt(packet.seqNumber);
    b.writeBigInt(packet.cNumber);
    b.writeUInt8(packet.label);
    b.writeBuffer(packet.data);
    assert(b.finished());
    return b.buffer;
}

function deserializeFullPacket(buf) {
    var b = new BufferManager(buf);
    var diaryNumber = b.readBigInt();
    var src = b.readBigInt();
    var dst = b.readBigInt();
    var seqNumber = b.readBigInt();
    var cNumber = b.readBigInt();
    var label = b.readUInt8();
    var data = b.getRemainingBuffer();
    
    var result = {
	diaryNumber: diaryNumber,
	src: src,
	dst: dst,
	seqNumber: seqNumber,
	cNumber: cNumber,
	label: label,
	data: data
    };
    return result;
}

function serialize(packet) {
    if (packet == undefined) {
	return new Buffer(0);
    } else if (isFullPacket(packet)) {
	return serializeFullPacket(packet);
    } else {
	return serializeLightPacket(packet);
    }
}

function deserialize(x) {
    if (x.length == 0) {
	return undefined;
    } else if (isSerializedLightPacket(x)) {
	return deserializeLightPacket(x);
    } else {
	return deserializeFullPacket(x);
    }
}

exports.isLightPacket = isLightPacket;
exports.isFullPacket = isFullPacket;
exports.hasDiaryNumber = hasDiaryNumber;
exports.serialize = serialize;
exports.deserialize = deserialize;

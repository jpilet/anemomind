var assert = require('assert');

var int64Size = 8;

// src and dst.
var mailboxIdSize = int64Size;

// cNumber, seqNumber, diaryNumber are various counters.
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

BufferManager.prototype.writeInt = function(x, elemSize) {
    assert(!this.finished());
    this.buffer.writeIntLE(x, this.pointer, elemSize);
    this.pointer += elemSize;
}

BufferManager.prototype.readInt = function(elemSize) {
    assert(!this.finished());    
    this.buffer.readIntLE(this.pointer, this.elemSize);
}


function serializeLight(packet) {
    assert(isLightPacket(packet));
    
     var dst = new BufferManager(
	2*counterSize + 2*mailboxIdSize
    );

    // To serialize: diaryNumber, src, dst, seqNumber       
    dst.writeInt(src.diaryNumber, counterSize);
    dst.writeInt(src.src, mailboxIdSize);
    dst.writeInt(src.dst, mailboxIdSize);
    dst.writeInt(src.seqNumber, counterSize);    

    assert(dst.finished());
    return dst;
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

exports.isLightPacket = isLightPacket;
exports.isFullPacket = isFullPacket;
exports.hasDiaryNumber = hasDiaryNumber;

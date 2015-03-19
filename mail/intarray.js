// Takes an array of integers and returns a buffer
function serialize(arr, elemsize) {
    var bytes = elemsize*arr.length;
    var dst = new Buffer(bytes);
    for (var i = 0; i < arr.length; i++) {
	buffer.writeIntLE(
	    arr[i],
	    elemsize*i,
	    elemsize);
    }
    return buffer;
}

// Takes a buffer and returns an array of integers
function deserialize(buffer, elemsize) {
    var count = Math.floor(buffer.length/elemsize);
    if (buffer.length - count*elemsize) {
	// Return undefined to signal that something went wrong.
	return undefined;
    }

    var dst = new Array(count);
    for (var i = 0; i < count; i++) {
	dst[i] = buffer.readIntLE(elemsize*i, elemsize);
    }
    return dst;
}

exports.serialize = serialize;
exports.deserialize = deserialize;

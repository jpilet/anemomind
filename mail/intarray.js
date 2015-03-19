function makeDefaultElemSize(x) {
    // Large enough for most integers, such as sequence numbers.
    return x || 8;
}

// Takes an array of integers and returns a buffer
function serialize(arr, elemsize) {
    elemsize = makeDefaultElemSize(elemsize);
    var bytes = elemsize*arr.length;
    var dst = new Buffer(bytes);
    
    for (var i = 0; i < arr.length; i++) {
	dst.writeIntLE( // <-- Requires recent (e.g. 0.12) version of node
	    arr[i],
	    elemsize*i,
	    elemsize);
    }
    return dst;
}

// Takes a buffer and returns an array of integers
function deserialize(buffer, elemsize) {
    elemsize = makeDefaultElemSize(elemsize);
    var count = Math.floor(buffer.length/elemsize);
    if (buffer.length - count*elemsize) {
	// Return undefined to signal that something went wrong.
	return undefined;
    }

    var dst = new Array(count);
    for (var i = 0; i < count; i++) {
	dst[i] = buffer.readIntLE(
	    elemsize*i, elemsize);  // <-- Requires recent (e.g. 0.12) version of node
    }
    return dst;
}


module.exports.serialize = serialize;
module.exports.deserialize = deserialize;

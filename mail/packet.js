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

exports.isLightPacket = isLightPacket;
exports.isFullPacket = isFullPacket;
exports.hasDiaryNumber = hasDiaryNumber;

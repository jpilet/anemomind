var boatIdPrefix = "boatId";
var boxIdPrefix = "boxId";

function makeMailboxNameFromBoatId(boatId) {
    if (typeof boatId == "number") {
	return boatIdPrefix + "_" + boatId;
    }
    return null;
}

function makeMailboxNameFromBoxId(boxId) {
    if (typeof boxId == "string") {
	return boxIdPrefix + "_" + boxId;
    }
    return null;
}

function isNumericType(type) {
    return type == boatIdPrefix;
}

function parseMailboxName(mailboxName) {
    if (typeof mailboxName == "string") {
	var index = mailboxName.indexOf("_");
	if (0 <= index) {
	    var type = mailboxName.substring(0, index);
	    var stringData = mailboxName.substring(index + 1);
	    var data = (isNumericType(type)? parseInt(stringData) : stringData);
	    var result = {};
	    result[type] = data;
	    return result;
	    
	}
	return null;
    }
    return null;
}


module.exports.makeMailboxNameFromBoatId = makeMailboxNameFromBoatId;
module.exports.makeMailboxNameFromBoxId = makeMailboxNameFromBoxId;
module.exports.parseMailboxName = parseMailboxName;

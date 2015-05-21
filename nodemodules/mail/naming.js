var types = {
  boatId: "string",
  boxId: "string",
};

function makeMailboxNameFromBoatId(boatId) {
  if (typeof boatId == types.boatId) {
    return "boatId_" + boatId;
  }
  return null;
}

function makeMailboxNameFromBoxId(boxId) {
  if (typeof boxId == types.boxId) {
    return "boxId_" + boxId;
  }
  return null;
}


function parseMailboxName(mailboxName) {
  if (typeof mailboxName == "string") {
    var index = mailboxName.indexOf("_");
    if (0 <= index) {
      var prefix = mailboxName.substring(0, index);
      if (types[prefix] != "string") {
	// not implemented for any other types than string.
	return null;
      }
      var stringData = mailboxName.substring(index + 1);
      var data = stringData;
      var result = {};
      result[prefix] = data;
      return result;
      
    }
    return null;
  }
  return null;
}


module.exports.makeMailboxNameFromBoatId = makeMailboxNameFromBoatId;
module.exports.makeMailboxNameFromBoxId = makeMailboxNameFromBoxId;
module.exports.parseMailboxName = parseMailboxName;

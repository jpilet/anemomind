function makeMailboxNameFromBoatId(boatId) {
  return "boat" + boatId;
}

function makeMailboxNameFromBoxId(boxId) {
  return "box" + boxId;
}


function parseMailboxName(mailboxName) {
  if (typeof mailboxName == "string") {
    var groups = mailboxName.match(/^(box|boat)(.*)/);
    if (groups) {
      return {prefix: groups[1], id: groups[2]};
    } else {
      return null;
    }
  }
  return null;
}


module.exports.makeMailboxNameFromBoatId = makeMailboxNameFromBoatId;
module.exports.makeMailboxNameFromBoxId = makeMailboxNameFromBoxId;
module.exports.parseMailboxName = parseMailboxName;

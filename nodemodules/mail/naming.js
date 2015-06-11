var path = require('path');

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


var filenameSuffix = '.mailsqlite.db';

function makeDBFilename(mailboxName) {
  return mailboxName + filenameSuffix;
}

function getMailboxNameFromFilename(fullFilename) {
  var parsed = path.parse(fullFilename);
  var filename = parsed.base;
  var index = filename.indexOf(filenameSuffix);
  if (0 < index) {
    if (index + filenameSuffix.length == filename.length) {
      return filename.substring(0, index);
    }
  }
  return null;
}

module.exports.makeMailboxNameFromBoatId = makeMailboxNameFromBoatId;
module.exports.makeMailboxNameFromBoxId = makeMailboxNameFromBoxId;
module.exports.parseMailboxName = parseMailboxName;
module.exports.makeDBFilename = makeDBFilename;
module.exports.getMailboxNameFromFilename = getMailboxNameFromFilename;

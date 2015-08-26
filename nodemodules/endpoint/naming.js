var path = require('path');

function makeEndpointNameFromBoatId(boatId) {
  return "boat" + boatId;
}

function makeEndpointNameFromBoxId(boxId) {
  return "box" + boxId;
}


function parseEndpointName(endpointName) {
  if (typeof endpointName == "string") {
    var groups = endpointName.match(/^(box|boat)(.*)/);
    if (groups) {
      return {prefix: groups[1], id: groups[2]};
    } else {
      return null;
    }
  }
  return null;
}


var filenameSuffix = '.mailsqlite.db';

function makeDBFilename(endpointName) {
  return endpointName + filenameSuffix;
}

function makeDBFilenameFromBoatId(boatId) {
  return makeDBFilename(makeEndpointNameFromBoatId(boatId));
}
  
function makeDBFilenameFromBoxId(boxId) {
  return makeDBFilename(makeEndpointNameFromBoxId(boxId));
}

function getEndpointNameFromFilename(fullFilename) {
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

module.exports.makeEndpointNameFromBoatId = makeEndpointNameFromBoatId;
module.exports.makeEndpointNameFromBoxId = makeEndpointNameFromBoxId;
module.exports.parseEndpointName = parseEndpointName;
module.exports.makeDBFilename = makeDBFilename;
module.exports.getEndpointNameFromFilename = getEndpointNameFromFilename;
module.exports.makeDBFilenameFromBoatId = makeDBFilenameFromBoatId;
module.exports.makeDBFilenameFromBoxId = makeDBFilenameFromBoxId;

bigint = require('./bigint.js');

module.exports.ack = 127;
module.exports.logfile = 128;
module.exports.scriptRequest = 129;
module.exports.scriptResponse = 130;

function isString(x) {
  return typeof x == 'string';
}

function isIdentifier(x) {
  if (typeof x == 'string') {
    // numbers and digits and some simple characters. At least one character.
    return !!x.match(/^[\w]+$/); 
  }
  return false;
}

function isCounter(x) {
  return bigint.isBigInt(x);
}

function isValidMailboxName(x) {
  return isIdentifier(x);
}

function isPacket(x) {
  if (typeof x == 'object') {
    return x.src && x.dst && x.label && x.seqNumber && x.data;
  }
  return false;
}

function makeBuf(x) {
  var dst = new Buffer(x.length);
  for (var i = 0; i < x.length; i++) {
    dst.writeUInt8(x[i], i);
  }
  return dst;
}

function isObjectWithFields(x, fields) {
  if (typeof x == 'object') {
    for (var i = 0; i < fields.length; i++) {
      if (!x.hasOwnProperty(fields[i])) {
	return false;
      }
      return true;
    }
  }
  return false;
}


module.exports.isCounter = isCounter; 
module.exports.isIdentifier = isIdentifier;
module.exports.isValidMailboxName = isValidMailboxName;
module.exports.isPacket = isPacket;
module.exports.isString = isString;
module.exports.isObjectWithFields = isObjectWithFields;
module.exports.makeBuf = makeBuf;

bigint = require('./bigint.js');

module.exports.ack = 127;
module.exports.file = 128;

function isIdentifier(x) {
  return typeof x == 'string';
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

module.exports.isCounter = isCounter; 
module.exports.isIdentifier = isIdentifier;
module.exports.isValidMailboxName = isValidMailboxName;
module.exports.isPacket = isPacket;

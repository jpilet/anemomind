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

module.exports.isCounter = isCounter; 
module.exports.isIdentifier = isIdentifier;
module.exports.isValidMailboxName = isValidMailboxName;

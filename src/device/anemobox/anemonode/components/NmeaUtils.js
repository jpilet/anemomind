var assert = require('assert');

var re = /\$GNRMC(,[^*]*)*\*[A-Z0-9]{2}/;

function onlyGNRMC(sentence) {
  var results = re.exec(sentence);
  if (!results) {
    return null;
  }
  return new Buffer(results[0] + "\r\n");
}

function notNull(r) {
  return r != null;
}

function sample(publishToPort) {
  return function(dst, chunk) {
    var lines = ('' + chunk).split('\n');
    var sentences = lines.map(onlyGNRMC).filter(notNull);
    return sentences.reduce(publishToPort, dst);
  };
}

module.exports.sample = sample;

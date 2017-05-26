var assert = require('assert');
var dr = require('./DataRate.js');
var su = require('./StringUtils.js');
var fn = require('./Functional.js');

var re = /\$([A-Z]{5})(,[^*]*)*\*[A-Z0-9]{2}/;

function valid(x) {
  return x != null;
}

function parseNmea(sentence) {
  var results = re.exec(sentence);
  if (!results) {
    return null;
  }
  return {
    full: sentence,
    clean: results[0],
    prefix: results[1]
  };
}

function toUpperCase(x) {
  return x.toUpperCase();
}

function prefixIn(prefices0) {
  var prefices = prefices0.map(toUpperCase);
  return function(parsedSentence) {
    return prefices.indexOf(parsedSentence.prefix.toUpperCase()) != -1;
  }
}

// This is the kind of eol produced by the chip.
var eol = "\r\n";

function getCleanSentence(x) {
  return new Buffer(x.clean + eol);
}

var sample = fn.compose(su.catSplit('\n'),
                        fn.map(parseNmea),
                        fn.filter(valid),
                        fn.filter(prefixIn(["GNRMC"])),
                        fn.map(getCleanSentence));

var maxBytesPerSecond = 100;
var windowLengthSeconds = 30;
var rateLimitAcceptor = dr.limitRateAcceptor(maxBytesPerSecond, 
                                             windowLengthSeconds);
var sampleLimited = fn.compose(sample, 
                               fn.filter(rateLimitAcceptor));

module.exports.sample = sample;
module.exports.sampleLimited = sampleLimited;

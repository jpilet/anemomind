var assert = require('assert');

function tag(value, unit) {
  if (value == null) {
    return null;
  }
  return [value, unit];
}

var knotsToSI = 0.514444;

var unitToSI = {
  "knots": knotsToSI,
  "kn": knotsToSI,
  "kt": knotsToSI,
  "knot": knotsToSI
};

function isTagged(x) {
  return x instanceof Array 
    && x.length == 2 
    && typeof x[1] == 'string';
}

function taggedToSI(tagged) {
  assert(isTagged(tagged));
  var quantity = tagged[0];
  var unit = tagged[1];
  assert(unit in unitToSI);
  return quantity*unitToSI[unit];
}

/*

Utility for executing something, but not 
too often. (e.g. to avoid flooding the standard output).

// Usage:

var limitWarn = makeTemporalLimiter(1000);

while (true) {
  limitWarn(function() {
    console.warn("We are in an infinite loop!");
  });
}

*/
function makeTemporalLimiter(minPeriod) {
  var lastTime = null;
  return function(bodyToExecute, timeOrNull) {
    var t = timeOrNull || new Date();
    if (lastTime == null || (t - lastTime) > minPeriod) {
      bodyToExecute();
      lastTime = t;
    }
  };
}

function getOrDefault(obj, key, defaultValue) {
  if (!obj || !(key in obj)) {
    return defaultValue;
  }
  return obj[key];
}

function nextSid(sid) {
  return (0 <= sid && sid < 255)? sid+1 : 0;
}

module.exports.tag = tag;
module.exports.makeTemporalLimiter = makeTemporalLimiter;
module.exports.getOrDefault = getOrDefault;
module.exports.nextSid = nextSid;
module.exports.taggedToSI = taggedToSI;

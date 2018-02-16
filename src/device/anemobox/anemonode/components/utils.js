function tag(value, unit) {
  if (value == null) {
    return null;
  }
  return [value, unit];
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
  return function(bodyToExecute) {
    var t = new Date();
    if (lastTime == null || (t - lastTime) > minPeriod) {
      bodyToExecute();
      lastTime = t;
    }
  };
}

module.exports.tag = tag;
module.exports.makeTemporalLimiter = makeTemporalLimiter;
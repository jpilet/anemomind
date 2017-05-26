var assert = require('assert');

var ms = 1000;

function NaiveRateLimiter(maxRateBytesPerSecond, resetPeriodSeconds) {
  this.sumBytes = 0;
  this.thresholdBytes = maxRateBytesPerSecond*resetPeriodSeconds;

  var self = this;
  setInterval(function() {
    self.sumBytes = 0;
  }, resetPeriodSeconds*ms);
};

NaiveRateLimiter.prototype.accept = function(bytes) {
  assert(typeof bytes == 'number');
  if (bytes.length > this.thresholdBytes) {
    console.log("WARNING: RateLimiter was queried on %d bytes, which always exceeds the threshold %d bytes",
                bytes, this.thresholdBytes);
  }

  var nextSum = this.sumBytes + bytes;
  if (nextSum <= this.thresholdBytes) {
    this.sumBytes = nextSum;
    return true;
  } else {
    return false;
  }
}

module.exports.NaiveRateLimiter = NaiveRateLimiter;
 

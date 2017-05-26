var assert = require('assert');

function RateEstimator(windowSize) {
  this.windowSize = windowSize;
  this.data = [];
  this.sum = 0;
  this.oldest = null;
}

RateEstimator.prototype.popOld = function(time) {
  var at = 0;
  while (at < this.data.length 
         && (time - this.data[at].time> this.windowSize)) {
    this.sum -= this.data[at].value;
    at++;
  }
  this.data = this.data.slice(at);
}

RateEstimator.prototype.add = function(time, value) {
  if (!this.oldest) {
    this.oldest = time;
  }
  // Because we maintain a sum, we only work
  // with integers to avoid round-off errors to accumulate.
  assert(Number.isInteger(value)); 

  this.data.push({time: time, value: value});
  this.sum += value;
  this.popOld(time);
}

RateEstimator.prototype.estimate = function() {
  var otherWindowSize = this.data.length == 0?
      0 : (this.data[this.data.length-1].time - this.oldest);
  var windowSize = Math.min(this.windowSize, otherWindowSize);
  return windowSize <= 0? Infinity : this.sum/windowSize;
}

var ms = 1000.0;

function limitRateAcceptor(maxBytesPerSecond, windowSizeSeconds) {
  var r = new RateEstimator(windowSizeSeconds*ms);
  return function(data) {
    r.add(new Date(), data.length);

    // The estimator returns bytes per millisecond,
    // (due to difference of two Date objects)
    // so we need to multiply by 1000 to get the actual rate
    var rate = r.estimate()*ms;
    var acceptanceProbability = maxBytesPerSecond/rate;
    return rate == 0 || (Math.random() < acceptanceProbability);
  };
}

module.exports.RateEstimator = RateEstimator;
module.exports.limitRateAcceptor = limitRateAcceptor;

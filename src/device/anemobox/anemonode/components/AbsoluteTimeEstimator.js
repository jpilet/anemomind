var OnlineMedianFinder = require('./OnlineMedianFinder.js');
var assert = require('assert');

function AbsoluteTimeEstimator(maxCount) {
  this.maxCount = maxCount;
  this.median = null;
  this.omf = new OnlineMedianFinder(function(a, b) {return a - b;});
};


AbsoluteTimeEstimator.prototype.addTimePair = function(systemTime, externalTime) {
  if (this.omf != null) {
    assert(systemTime instanceof Date);
    assert(externalTime instanceof Date);
    var dif = externalTime.getTime() - systemTime.getTime();
    console.log("Time dif: " + dif);
    this.omf.addElement(dif);
    this.median = this.omf.currentMedian;
    console.log("Median: " + this.median);
  }
  if (this.omf.size() >= this.maxCount) {
    this.omf = null;    
  }
}

AbsoluteTimeEstimator.prototype.estimateCurrentTime = function(systemTime) {
  return this.median == null?
    null : new Date(systemTime.getTime() + this.median);
}


// Now-methods
AbsoluteTimeEstimator.prototype.addExternalTimeNow = function(externalTime) {
  this.addTimePair(new Date(), externalTime);
}

AbsoluteTimeEstimator.prototype.estimateCurrentTimeNow = function() {
  return this.estimateCurrentTime(new Date());
}

module.exports = AbsoluteTimeEstimator;

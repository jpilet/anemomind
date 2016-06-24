var OnlineMedianFinder = require('./OnlineMedianFinder.js');

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
    this.omf.addElement(dif);
    this.median = this.omf.currentMedian;
  }
  if (this.omf.size() >= this.maxCount) {
    this.omf = null;    
  }
}

AbsoluteTimeEstimator.prototype.estimateCurrentTime = function(systemTime) {
  return this.median == null?
    null : return new Date(systemTime.getTime() + this.median);
}


// Now-methods
AbsoluteTimeEstimator.prototype.addExternalTimeNow = function(externalTime) {
  this.addTimePair(new Date(), externalTime);
}

AbsoluteTimeEstimator.prototype.estimateCurrentTimeNow = function() {
  return this.estimateCurrentTime(new Date());
}

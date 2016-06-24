var AbsoluteTimeEstimator = require('./AbsoluteTimeEstimator');
var anemonode = require('../build/Release/anemonode');

var estimator = new AbsoluteTimeEstimator(300);

function updateEstimatorFromDispatcher() {
  var sysTime = anemonode.dispatcher.values.dateTime.time(0);
  var gpsTime = anemonode.dispatcher.values.dateTime.value(0);
  if (!sysTime || !gpsTime) {
    console.log('time undefined');
    return;
  }
  estimator.addTimePair(sysTime, gpsTime);
}
anemonode.dispatcher.values.dateTime.subscribe(updateEstimatorFromDispatcher);

module.exports = function() {return estimator.estimateCurrentTimeNow() || new Date();}

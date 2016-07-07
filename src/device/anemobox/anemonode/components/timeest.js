var assert = require('assert');
var anemonode = require('../build/Release/anemonode');

function medianDeltaTime(src, hlen) {
  assert(src.length);
  assert(src.value);
  assert(src.time);n
  assert(typeof(hlen) == 'number');
  var n = Math.min(hlen, src.length());
  var deltas = [];
  for (var i = 0 ; i < n; ++i) {
    var sysTime = src.time(i);
    var gpsTime = src.value(i);
    if (sysTime && gpsTime) {
      deltas.push(gpsTime - sysTime);
    }
  }
  if (deltas.length > 0) {
    deltas.sort();
    return deltas[Math.floor(deltas.length / 2)];
  }
  return undefined;
}

function estimateTime(src, hlen) {
  if (src.length() == 0) {
    return new Date();
  } else {
    var sys = src.time(0);
    var offset = medianDeltaTime(src, hlen);
    if (offset == null) {
      return sys;
    } else {
      return new Date(sys.getTime() + offset);
    }
  }
}

var historyLength = 60;

function estimateTimeFromDispatcher() {
  return estimateTime(anemonode.dispatcher.value.dateTime, historyLength);
}

module.exports.estimateTimeFromDispatcher = estimateTimeFromDispatcher;
module.exports.medianDeltaTime = medianDeltaTime;
module.exports.estimateTime = estimateTime;

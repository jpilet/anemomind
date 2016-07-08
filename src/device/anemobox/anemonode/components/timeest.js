var assert = require('assert');
var anemonode = require('../build/Release/anemonode');


// Settings
var cacheExpiryThreshold = 1000;
var historyLength = 60;



function medianDeltaTime(src, hlen) {
  assert(src.length);
  assert(src.value);
  assert(src.time);
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


var cache = null;

function medianDeltaTimeMemoized(src, hlen, t) {
  if (cache == null || Math.abs(cache.time - t) > cacheExpiryThreshold) {
    cache = {
      time: t,
      offset: medianDeltaTime(src, hlen)
    };
  }
  return cache.offset;
}


function estimateTime(src, hlen) {
  if (src.length() == 0) {
    return new Date();
  } else {
    var sys = src.time(0);
    var offset = medianDeltaTimeMemoized(src, hlen, sys);
    if (offset == null) {
      return sys;
    } else {
      return new Date(sys.getTime() + offset);
    }
  }
}

function estimateTimeFromDispatcher() {
  return estimateTime(anemonode.dispatcher.value.dateTime, historyLength);
}

module.exports.estimateTimeFromDispatcher = estimateTimeFromDispatcher;
module.exports.medianDeltaTime = medianDeltaTime;
module.exports.estimateTime = estimateTime;

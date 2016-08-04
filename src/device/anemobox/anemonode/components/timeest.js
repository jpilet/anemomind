var assert = require('assert');
var anemonode = require('../build/Release/anemonode');
var timesrc = require('endpoint/timesrc.js')


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


function estimateTime(currentSystemTime, src, hlen) {
  assert(currentSystemTime instanceof Date);
  if (src.length() == 0) {
    return currentSystemTime;
  } else {
    var offset = medianDeltaTimeMemoized(src, hlen, currentSystemTime);
    if (offset == null) {
      return currentSystemTime;
    } else {
      return new Date(currentSystemTime.getTime() + offset);
    }
  }
}

function estimateTimeFromDispatcher() {
  return estimateTime(new Date(), anemonode.dispatcher.value.dateTime, historyLength);
}

timesrc.get = estimateTimeFromDispatcher;

module.exports.estimateTimeFromDispatcher = estimateTimeFromDispatcher;
module.exports.medianDeltaTime = medianDeltaTime;
module.exports.estimateTime = estimateTime;

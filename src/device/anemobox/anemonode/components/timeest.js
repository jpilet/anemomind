var assert = require('assert');
var anemonode = require('../build/Release/anemonode');
var timesrc = require('endpoint/timesrc.js')
var prodassert = require('./prodassert.js').prodassert;

// Settings
var cacheExpiryThreshold = 1000;
var historyLength = 60;



function medianDeltaTime(src, hlen) {
  prodassert(src.length);
  prodassert(src.value);
  prodassert(src.time);
  prodassert(typeof(hlen) == 'number');
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
  prodassert(currentSystemTime instanceof Date);
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
  return estimateTime(anemonode.currentTime(),
                      anemonode.dispatcher.values.dateTime,
                      historyLength);
}

timesrc.now = estimateTimeFromDispatcher;

module.exports.estimateTimeFromDispatcher = estimateTimeFromDispatcher;
module.exports.medianDeltaTime = medianDeltaTime;
module.exports.estimateTime = estimateTime;
module.exports.now = estimateTimeFromDispatcher;

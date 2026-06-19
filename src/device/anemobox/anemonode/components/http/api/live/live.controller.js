'use strict';

var anemonode = require('../../../../build/Release/anemonode');
var timeest = require('../../../timeest.js');

var pendingCalls = {}
var pendingCallPackets = [];

var lastFetch;

var HISTORY_MAX_MS = 30 * 60 * 1000;  // window returned by the history endpoint

// Return per-channel history read directly from the dispatcher's ring buffer.
// The dispatcher keeps a 30-minute window per channel in RAM.
// Index 0 is the most recent sample, increasing indices go further back.
// Timestamps are in the monotonic clock; `now` lets the client correct for
// any clock skew between the box and the browser.
//
// Optional query parameters:
//   channels=a,b,c   only return these channels (default: all numeric ones)
//   duration=<sec>   only return samples newer than <sec> seconds ago
//                    (default and maximum: the 30-minute buffer window)
exports.history = function(req, res) {
  var now = anemonode.currentTime().getTime();

  var maxAgeMs = HISTORY_MAX_MS;
  if (req.query.duration != undefined) {
    var durSec = parseFloat(req.query.duration);
    if (!isNaN(durSec) && durSec > 0) {
      maxAgeMs = Math.min(HISTORY_MAX_MS, durSec * 1000);
    }
  }

  var wanted = null;
  if (req.query.channels != undefined) {
    wanted = {};
    req.query.channels.split(',').forEach(function(c) {
      c = c.trim();
      if (c) {
        wanted[c] = true;
      }
    });
  }

  var channels = {};
  for (var i in anemonode.dispatcher.values) {
    if (wanted && !wanted[i]) {
      continue;
    }
    var val = anemonode.dispatcher.values[i];
    var n = val.length();
    if (n === 0) {
      continue;
    }
    var samples = [];
    for (var k = 0; k < n; ++k) {
      var v = val.value(k);
      // Only numeric channels are charted; skip pos/orient/date/binary.
      if (typeof v !== 'number' || isNaN(v)) {
        break;
      }
      var t = val.time(k).getTime();
      if (now - t > maxAgeMs) {
        break;  // samples are ordered newest-first, so we can stop here
      }
      samples.push({ t: t, v: v });
    }
    if (samples.length > 0) {
      samples.reverse();  // oldest first
      channels[i] = samples;
    }
  }
  res.json({ now: now, channels: channels });
};

// Get list of values, only from the best source per channel
exports.index = function(req, res) {
  var response = {};
  var monotonicTime = anemonode.currentTime();
  for (var i in anemonode.dispatcher.values) {
    if (anemonode.dispatcher.values[i].length() > 0
        && Math.abs(anemonode.dispatcher.values[i].time().getTime() - monotonicTime.getTime()) < 2000) {
      response[i] = {
        v: anemonode.dispatcher.values[i].value(),
        s: anemonode.dispatcher.values[i].source()
      };
    }
  }
  if (pendingCallPackets.length > 0) {
    response.rpcCalls = pendingCallPackets;
    pendingCallPackets = [];
  }
  lastFetch = monotonicTime;
  res.json(response);
};

exports.allSources = function(req, res) {
  var response = {};
  var sources = anemonode.dispatcher.allSources();

  for (var channel in sources) {
    var sourcesForChannel = { }
    for (var source in sources[channel]) {
      sourcesForChannel[source] = {
        v: sources[channel][source].value(),
        t: timeest.monotonicToEstimatedTime(sources[channel][source].time()),
        p: anemonode.dispatcher.sourcePriority(source)
      };
    }
    response[channel] = sourcesForChannel;
  }

  res.json(response);
}


function handleError(res, err) {
  return res.status(500).send(err);
}


exports.callRpc = function(func, args, callback) {
  var packet = {
    callId: Math.round(Math.random() * 65535),
    func: func,
    args: args
  };

  pendingCalls[packet.callId] = callback || function() {};

  pendingCallPackets.push(packet);
}

exports.rpcReply = function(req, res) {
  var callId = req.params.callId;
  if (callId == undefined) {
    return res.status(403).send("no callId");
  }
  if (!(callId in pendingCalls)) {
    return res.status(404).send("No such callId");
  }
  
  var answer = ('answer' in req.body ? req.body.answer : undefined);
  pendingCalls[callId](answer);

  res.status(200).send();
}

exports.isConnected = function() {
  if (!lastFetch) {
    return false;
  }
  return (anemonode.currentTime().getTime() - lastFetch.getTime()) < 5000;
}

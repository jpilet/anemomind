'use strict';

var anemonode = require('../../../../build/Release/anemonode');

var pendingCalls = {}
var pendingCallPackets = [];

var lastFetch;

// Get list of values, only from the best source per channel
exports.index = function(req, res) {
  var response = {};
  var date = (new Date()).getTime();
  for (var i in anemonode.dispatcher.values) {
    if (anemonode.dispatcher.values[i].length() > 0
        && Math.abs(anemonode.dispatcher.values[i].time().getTime() - date) < 2000) {
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
  lastFetch = new Date()
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
        t: sources[channel][source].time(),
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
  var now = new Date();
  return (now.getTime() - lastFetch.getTime()) < 5000;
}

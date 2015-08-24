'use strict';

var anemonode = require('../../../../build/Release/anemonode');

// Get list of boats
exports.index = function(req, res) {
  var response = {};
  for (var i in anemonode.dispatcher.values) {
    var date = new Date();
    if (anemonode.dispatcher.values[i].length() > 0 && Math.abs(anemonode.dispatcher.values[i].time() - date) < 1000) {
      response[i] = anemonode.dispatcher.values[i].value()
    }
  }
  res.json(response);
};

function handleError(res, err) {
  return res.status(500).send(err);
}

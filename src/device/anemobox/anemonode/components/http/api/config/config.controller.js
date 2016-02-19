'use strict';

var os = require('os');
var anemonode = require('../../../../build/Release/anemonode');
var config = require('../../../../components/config');

var getNetworkInfo() {
  var ifaces = os.networkInterfaces();
  if ("wlan0" in ifaces) {
    for (var i in ifaces.wlan0) {
      if (ifaces.wlan0[i].family == 'IPv4') {
        return ifaces.wlan0[i];
      }
    }
  }
  return undefined;
}

// Get list of boats
exports.index = function(req, res) {

  config.get(function(err, cfg) {
    var response = {};

    response.date = (new Date()).getTime();

    if (err) {
      response.boxConfigError = err;
    } else {
      response.boxConfig = cfg;
    }

    response.networkInfo = getNetworkInfo();
    res.json(response);
  });
};


'use strict';

var os = require('os');
var anemonode = require('../../../../build/Release/anemonode');
var config = require('../../../../components/config');
var version = require('../../../../version');
var now = require('../../../timeest.js').now;

function getNetworkInfo() {
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

// Returns a configuration summary, for example:
//    { date: 1455897874242,
//     boxConfig: { boatName: '', boatId: '', nmea0183Speed: 4800 },
//     networkInfo: 
//      { address: '192.168.1.109',
//        netmask: '255.255.255.0',
//        family: 'IPv4',
//        mac: '78:4b:87:aa:2c:ca',
//        internal: false } }
exports.index = function(req, res) {

  config.get(function(err, cfg) {
    var response = {};

    response.date = now().getTime();

    if (err) {
      response.boxConfigError = err;
    } else {
      response.boxConfig = cfg;
    }

    response.networkInfo = getNetworkInfo();
    response.version = version.string;

    res.json(response);
  });
};

// Expects a key/value dictionary of what to change
exports.changeConfig = function(req, res) {
  config.change(req.body, function(err, result) {
    if (err) {
      res.status(500).send("changeConfig error: " + err);
    } else {
      res.status(200).json(result)
    }
  });
};

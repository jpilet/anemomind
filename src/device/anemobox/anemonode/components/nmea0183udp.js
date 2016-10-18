var anemonode = require('../build/Release/anemonode');

var dgram = require('dgram');

var nmeaParsers = {};
var server;
var currentPort;

function listenToUdpPort(port, dataCB) {
  port = parseInt(port);

  if (currentPort && currentPort == port) {
    // already listening on the same port.
    return;
  }

  var open = function() {
    if (port) {
      openPort(port, dataCB);
    }
  };

  if (server) {
    server.on('close', open);
    server.close();
  } else {
    open();
  }
}

function openPort(port, dataCB) {
  currentPort = port;
  server = dgram.createSocket('udp4');

  server.on('error', function (err) {
    console.log('server error: ' + err.stack);
    server.close();
  });

  server.on('message', function(msg, rinfo) {
    var source = 'NMEA0183 ' + rinfo.address + ':' + rinfo.port + ' UDP';

    if (!(source in nmeaParsers)) {
      nmeaParsers[source] = new anemonode.Nmea0183Source(source);
    }

    nmeaParsers[source].process(msg);

    if (dataCB) {
      dataCB(source, msg);
    }
  });

  server.on('listening', function() {
              var address = server.address();
                console.log("listening for NMEA0183 on UDP: " + address.address + ":" + address.port);
                });

  server.bind(port);
}

module.exports.listenToUdpPort = listenToUdpPort;

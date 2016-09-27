var anemonode = require('../build/Release/anemonode');

var dgram = require('dgram');

var nmeaParsers = {};
var server;
var currentPort;

function listenToUdpPort(port, dataCB) {
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
    server.close(openPort);
  } else {
    openPort();
  }
}

function openPort(port, dataCB) {
  currentPort = port;
  server = dgram.createSocket('udp4');

  server.on('error', (err) => {
    console.log('server error: ' + err.stack);
    server.close();
  });

  server.on('message', (msg, rinfo) => {
    var source = 'NMEA0183 ' + rinfo.address + ':' + rinfo.port + ' UDP';

    if (!(source in nmeaParsers)) {
      nmeaParsers[source] = new anemonode.Nmea0183Source(source);
    }

    nmeaParsers[source].process(msg);

    if (dataCB) {
      dataCB(source, msg);
    }
  });

  server.on('listening', () => {
              var address = server.address();
                console.log(`server listening ${address.address}:${address.port}`);
                });

  server.bind(port);
}

module.exports.listenToUdpPort = listenToUdpPort;

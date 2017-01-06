var nmea0183udp = require('./components/nmea0183udp.js');

var port = 50000;

nmea0183udp.listenToUdpPort(
      port,
      function(source, data) { 
        if (withLogger && logExternalNmea) {
          logger.logText(source, data.toString('ascii'));
        }
      }
  );


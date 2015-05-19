var nmea0183PortPath = '/dev/ttyMFD1';
var logRoot = '/media/sdcard/logs/';
var logInterval = 5 * 60 * 1000;  // create a log file every 5 minutes

var config = require('./components/config');

require('./components/AnemoServiceBTLE').startBTLE();

// Initialize the NMEA0183 port
var nmea0183port = require('./components/nmea0183port');
nmea0183port.init(nmea0183PortPath);

// Internal GPS with output to NMEA0183
require('./components/gps').init(nmea0183port.emitNmea0183Sentence);

require('./components/logger').startLogging(logRoot, logInterval, function(path) {
  // TODO: send file to phone.
  console.log('log written to: ' + path);
});

require('./components/RpcAssignBoat');

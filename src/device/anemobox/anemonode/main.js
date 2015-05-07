var nmea0183PortPath = '/dev/ttyMFD1';
var logRoot = '/media/sdcard/logs';
var logInterval = 5 * 60 * 1000;  // create a log file every 5 minutes

var config = require('./components/config');

require('./components/AnemoServiceBTLE').startBTLE();
require('./components/gps').init();
require('./components/nmea0183port').init(nmea0183PortPath);

require('./components/logger').startLogging(logRoot, logInterval, function(path) {
  // TODO: send file to phone.
  console.log('log written to: ' + path);
});


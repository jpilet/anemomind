var nmea0183PortPath = '/dev/ttyMFD1';
var logRoot = '/media/sdcard/logs/';
var logInterval = 5 * 60 * 1000;  // create a log file every 5 minutes
var localMailbox = require('./components/LocalMailbox.js');
var sync = require('./components/sync.js');

localMailbox.postRemainingLogFiles(logRoot, function(err, files) {
  if (0 < files.length) {
    console.log('Posted these logfiles at startup:');
    console.log(files);
  }
});

var config = require('./components/config');
var logger = require('./components/logger');

require('./components/AnemoServiceBTLE').startBTLE();

// Initialize the NMEA0183 port
var nmea0183port = require('./components/nmea0183port');
nmea0183port.init(nmea0183PortPath,
                  function(data) { logger.logText("NMEA0183 input", data.toString('ascii')); });

// Internal GPS with output to NMEA0183
require('./components/gps').init(
    function(data) {
      nmea0183port.emitNmea0183Sentence(data);
      logger.logText("Internal GPS NMEA", data.toString('ascii'));
    });

// Set the system clock to GPS time
require('./components/settime.js');

require('./components/logger').startLogging(logRoot, logInterval, function(path) {
  localMailbox.postLogFile(path, function(err, remaining) {
    if (err) {
      console.log('###### Error posting file located at ' + path + ':');
    } else {
      sync.triggerSync();
      console.log('Posted this file: ' + path);
    }
  });
});

require('./components/RpcAssignBoat');


// The estimator computes the true wind and the target speed.
var estimator = require('./components/estimator.js');
estimator.loadCalib();
estimator.start();

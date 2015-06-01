var nmea0183PortPath = '/dev/ttyMFD1';
var logRoot = '/media/sdcard/logs/';
var logInterval = 5 * 60 * 1000;  // create a log file every 5 minutes

var withLocalMailbox = true;
var withLogger = true;
var withGps = true;
var withSetTime = true;
var withBT = true;
var echoGpsOnNmea = false;

var config = require('./components/config');

if (withLogger) {
  var logger = require('./components/logger');
}

if (withLocalMailbox) {
  var localMailbox = require('./components/LocalMailbox.js');

  localMailbox.postRemainingLogFiles(logRoot, function(err, files) {
    if (0 < files.length) {
      console.log('Posted these logfiles at startup:');
      console.log(files);
    }
  });
}

if (withBT) {
  require('./components/AnemoServiceBTLE').startBTLE();
}

// Initialize the NMEA0183 port
var nmea0183port = require('./components/nmea0183port');
nmea0183port.init(nmea0183PortPath,
  function(data) { 
  if (withLogger) {
    logger.logText("NMEA0183 input", data.toString('ascii'));
  }
});

// Internal GPS with output to NMEA0183
if (withGps) {
require('./components/gps').init(
    function(data) {
      if (echoGpsOnNmea) {
        nmea0183port.emitNmea0183Sentence(data);
      }
      if (withLogger) {
        logger.logText("Internal GPS NMEA", data.toString('ascii'));
      }
    });
}

// Set the system clock to GPS time
if (withSetTime) {
  require('./components/settime.js');
}

if (withLogger) {
  logger.startLogging(logRoot, logInterval, function(path) {
    if (withLocalMailbox) {
      localMailbox.postLogFile(path, function(err, remaining) {
        if (err) {
          console.log('###### Error posting file located at ' + path + ':');
        } else {
          console.log('Posted this file: ' + path);
        }
      });
    }
  });
}

require('./components/RpcAssignBoat');


// The estimator computes the true wind and the target speed.
if (0) {
  var estimator = require('./components/estimator.js');

  estimator.loadCalib();
  estimator.start();
}

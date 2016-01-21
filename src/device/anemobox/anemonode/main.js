var dispatcher = require('./build/Release/anemonode').dispatcher;

var nmea0183PortPath = '/dev/ttyMFD1';
var logRoot = '/media/sdcard/logs/';
var logInterval = 5 * 60 * 1000;  // create a log file every 5 minutes
var btrpcFuncTable = require('./components/rpcble.js').rpcFuncTable;
var withLocalEndpoint = true;
var withLogger = true;
var withGps = true;
var withSetTime = true;
var withBT = false;
var echoGpsOnNmea = false;
var withEstimator = true;
var logInternalGpsNmea = false;
var logExternalNmea = true;
var withHttp = true;
var withIMU = true;
var withCUPS = true;
var withNMEA2000 = true;

var config = require('./components/config');

if (withHttp) {
  var http = require('./components/http');
}

if (withLogger) {
  var logger = require('./components/logger');
}

if (withLocalEndpoint) {
  var localEndpoint = require('./components/LocalEndpoint.js');
  var sync = require('./components/sync.js');

  localEndpoint.postRemainingLogFiles(logRoot, function(err, files) {
    if (err) {
      console.log('Failed to post logfiles at startup:');
      console.log(err);
    } else if (files && 0 < files.length) {
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
  if (withLogger && logExternalNmea) {
    logger.logText("NMEA0183 input", data.toString('ascii'));
  }
});

// The default priority is 0. Setting nmea to -1 gives priority to
// internal GPS and to internally computed true wind information.
dispatcher.setSourcePriority(nmea0183port.sourceName(), -1);

// Lower internal GPS priority: external GPS is more relevant.
dispatcher.setSourcePriority("Internal GPS", -2);

// The CUPS sensor has less priority than NMEA.
dispatcher.setSourcePriority("CUPS", -2);

// Internal GPS with output to NMEA0183
var gps = (withGps ?  require('./components/gps') : {readGps:function(){}});
function gpsData(data) {
  if (echoGpsOnNmea) {
    nmea0183port.emitNmea0183Sentence(data);
  }
  if (withLogger && logInternalGpsNmea) {
    logger.logText("Internal GPS NMEA", data.toString('ascii'));
  }
}

if (withIMU) {
  var bno055 = require('./components/bno055.js');
}

function startI2CPolling() {
  if (withGps || withIMU) {
    // All I2C polling occur is triggered by this single timer.
    setInterval(function() {
      gps.readGps(gpsData);

      if (bno055) {
        bno055.readImu();
      }
    }, 80);
  }
}

if (withIMU) {
  bno055.init(startI2CPolling);
} else {
  startI2CPolling();
}


// Set the system clock to GPS time
if (withSetTime) {
  require('./components/settime.js');
}

if (withLogger) {
  logger.startLogging(logRoot, logInterval, function(path) {
    if (withLocalEndpoint) {
      localEndpoint.postLogFile(path, function(err, remaining) {
        if (err) {
          console.log('###### Error posting file located at ' + path + ':');
        } else {
          console.log('Posted this log file: ' + path + ". Triggering phone sync.");
          sync.triggerSync(function() {});
        }
      });
    }
  });
}

require('./components/RpcAssignBoat').register(btrpcFuncTable);
if (withLocalEndpoint) {
  require('./components/RpcEndpoint.js').register(btrpcFuncTable);
}

// The estimator computes the true wind and the target speed.
if (withEstimator) {
  var estimator = require('./components/estimator.js');

  estimator.loadCalib();
  estimator.start();
}

if (withCUPS) {
  require('./components/cups.js');
}

if (withNMEA2000) {
  require('./components/nmea2000.js');
}

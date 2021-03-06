
var dispatcher = require('./build/Release/anemonode').dispatcher;
var version = require('./version');
var logRoot = '/media/sdcard/logs/';
console.log('Anemobox firmware version ' + version.string);

var nmea0183PortPath = '/dev/ttyMFD1';
var logInterval = 5 * 60 * 1000;  // create a log file every 5 minutes
var withLocalEndpoint = true;
var withLogger = true;
var withGps = true;
var withTimeEstimator = true;
var withSetTime = true;
var withBT = false;
var echoGpsOnNmea = true;
var withEstimator = true;
var logInternalGpsNmea = false;
var logExternalNmea = true;
var withHttp = true;
var withIMU = true;
var withCUPS = false;
var withNMEA2000 = true;
var withWatchdog = !process.env['NO_WATCHDOG'];
var withFakeReplay = process.env['FAKE_REPLAY'];
var withCalypsoUltrasonic = true;
var withNmea0183Perf = true;

var withSendN2kGps = true;

var spiBugDetected = false;

var config = require('./components/config');
var reboot = require('./components/reboot').reboot;
var settings = require('./components/GlobalSettings.js');
var dof = require('./components/deleteOldFiles.js');
var nmea2000 = require('./components/nmea2000.js');
var n2kgps = require('./components/n2kgps.js');

if (withFakeReplay) {
  console.log('With fake replay');
  var fakeReplay = require('./components/fakereplay.js');
  fakeReplay.start();
}

// To free up space if possible.
function cleanOld() {
  dof.easyCleanFolder(settings.sentLogsPath, function(err) {
    console.log("Old files cleanup failed");
    console.log(err);
  });
}

// On startup, but also later, when we post files.
cleanOld();

var btrpcFuncTable = {};
if (withBT) {
  btrpcFuncTable = require('./components/rpcble.js').rpcFuncTable;
}

if (withHttp) {
  var http = require('./components/http');
}

function setLogRawNmea2000FromConfig(err, cfg) {
  if (err) {
    console.log("Error reading config setting to set log raw NMEA 2000 state");
  } else {
    nmea2000.setRawLogging(cfg.logRawNmea2000);
  }
}

if (withLogger) {
  var logger = require('./components/logger');
  config.getAndListen(setLogRawNmea2000FromConfig);
}

if (withLocalEndpoint) {
  var localEndpoint = require('./components/LocalEndpoint.js');
  var sync = require('./components/sync.js');
  function postRemaining(context) {
    localEndpoint.postRemainingLogFiles(logRoot, function(err, files) {
      if (err) {
        console.log('Failed to post logfiles at "%s":', context);
        console.log(err);
      } else if (files && 0 < files.length) {
        console.log('Posted these logfiles at "%s":', context);
        console.log(files);
      }
    });
  }
  config.events.on('change', function() {
    postRemaining('config change');
  });
  postRemaining('startup');
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

// NMEA2000 unknown devices, with id=0, have a very low priority.
dispatcher.setSourcePriority("NMEA2000/0", -10);

// Internal GPS with output to NMEA0183
var gps = (withGps ?  require('./components/gps') : {readGps:function(){}});
var rmcExpr = /\$GN((RMC)|(VTG))(,[^*]*)*\*[A-Z0-9]{2}/g;

function gpsData(data) {
  if (echoGpsOnNmea) {
    var s = (data.toString('ascii')).match(rmcExpr).join('\r\n')+'\r\n';
    nmea0183port.emitNmea0183Sentence(s);
  }
  if (withLogger && logInternalGpsNmea) {
    logger.logText("Internal GPS NMEA", data.toString('ascii'));
  }
  if (withSendN2kGps) {
    nmea2000.sendPackets(n2kgps.makePackets(data.toString()));
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

var getCurrentTime = withTimeEstimator? 
    require('./components/timeest.js').estimateTimeFromDispatcher 
    : function() {return new Date();};

function startLogging() {
  logger.startLogging(logRoot, logInterval, getCurrentTime, function(path) {
    cleanOld();
    if (withLocalEndpoint) {
      localEndpoint.postLogFile(path, function(err, remaining) {
        if (err) {
          console.log('###### Error posting file located at ' + path + ':');
        } else {
          console.log('Posted this log file: ' + path + ". Triggering phone sync.");
          sync.triggerSync(function() {});
        }
        if (spiBugDetected) {
          reboot();
        }
      });
    }
  });
}

if (withLogger) {
  startLogging();
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
  nmea2000.detectSPIBug(function() {
    var message = 'SPI bug detected, rebooting!';
    console.log(message);
    spiBugDetected = true;
    if (withLogger) {
      logger.logText("syslog", message);
      logger.flush();
    }
  });
  config.get(function(err, cfg) {
    nmea2000.startNmea2000(cfg);
  });
  config.getAndListen(function(err, cfg) {
    if (err) {
      console.warn("Got an error when refreshing the NMEA2000 state from config");
    } else if (cfg) {
      nmea2000.updateFromConfig(cfg);
    }
  });
}

if (withWatchdog) {
  require('./components/watchdog.js').startWatchdog();
}

var callrpc = require('./components/callrpc.js');
callrpc.WITH_BT = withBT;
callrpc.WITH_HTTP = withHttp;

if (withCalypsoUltrasonic) {
  var ultrasonic = require('./components/ultrasonic');

  config.getAndListen(function(err, cfg) {
    if (cfg) {
      if (cfg.calypsoUltrasonic) {
        ultrasonic.start();
      } else {
        ultrasonic.shutdown();
      }
    }
  });
}

if (withNmea0183Perf) {
  var nmea0183perf = require('./components/nmea0183perf');
  config.getAndListen(function(err, cfg) {
    if (cfg) {
      nmea0183perf.activateNmea0183PerfOutput(
         function(s) { nmea0183port.emitNmea0183Sentence(s); },
         cfg.nmea0183PerfOutFormat);
    }
  });
}

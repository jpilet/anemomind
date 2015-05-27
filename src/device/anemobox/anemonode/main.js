var nmea0183PortPath = '/dev/ttyMFD1';
var logRoot = '/media/sdcard/logs/';
var logInterval = 5 * 60 * 1000;  // create a log file every 5 minutes
var localMailbox = require('./components/LocalMailbox.js');

var config = require('./components/config');

require('./components/AnemoServiceBTLE').startBTLE();

// Initialize the NMEA0183 port
var nmea0183port = require('./components/nmea0183port');
nmea0183port.init(nmea0183PortPath);

// Internal GPS with output to NMEA0183
require('./components/gps').init(nmea0183port.emitNmea0183Sentence);

// Set the system clock to GPS time
require('./components/settime.js');

require('./components/logger').startLogging(logRoot, logInterval, function(path) {
  localMailbox.postLogFile(path, function(err) {
    if (err) {
      console.log('###### Error posting file located at ' + path + ':');
      console.log(err);
    }
  });
});

require('./components/RpcAssignBoat');

require('./components/RpcMailbox').fillTable(
    require('./components/rpcble').rpcFuncTable
);

// The estimator computes the true wind and the target speed.
var estimator = require('./components/estimator.js');
estimator.loadCalib();
estimator.start();

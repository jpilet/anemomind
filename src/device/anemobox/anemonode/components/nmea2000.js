var buffer = require('buffer');
var can = require('socketcan');
var anemonode = require('../build/Release/anemonode');
var exec = require('child_process').exec;
var logger = require('./logger.js');
var boxId = require('./boxId.js');
var version = require('../version.js');

var nmea2000;
var nmea2000Source;
var rawPacketLoggingEnabled = false;

boxId.getAnemoId(function(boxid) {
  var virtDevBits = 2;
  var maxNumVirtualDevices = 1 << virtDevBits;
  var baseSerialNumber =
    (parseInt(boxid, 16) & ((1 << 21 + 1 - virtDevBits) - 1)) << virtDevBits;

  nmea2000 = new anemonode.NMEA2000([
      {
        // product:
          serialCode: boxid,
          productCode: 140,
          model:"Anemobox logger",
          softwareVersion: '' + version.string,
          modelVersion: 'Anemobox v1',
          loadEquivalency : 3,
          nmea2000Version: 2101,
          certificationLevel: 0xff,
        // device:
          uniqueNumber: baseSerialNumber + 0,
          manufacturerCode: 2040,
          deviceFunction: 140, // traffic logger
          deviceClass: 10,
          source: 42
      }]);

  nmea2000Source = new anemonode.Nmea2000Source(nmea2000);

  nmea2000.setSendCanFrame(function(id, data) {
    var msg = { id: id, data: data, ext: true };
    var r = channel.send(msg);
    return r > 0;
  });
});

function canPacketReceived(msg) {
  logRawPacket(msg);
  if (nmea2000) {
    nmea2000.pushCanFrame(msg);
  }
}

function logRawPacket(msg) {
  if (rawPacketLoggingEnabled) {
    var systemTime0 = 1000*msg.ts_sec + 0.001*msg.ts_usec;
    var systemTime1 = new Date().getTime();
    var monotonicTime1 = anemonode.currentTime();
    
    // Solve this EQ: 
    // monotonicTime1 - monotonicTime0 = systemTime1 - systemTime0
    monotonicTime0 = monotonicTime1 - (systemTime1 - systemTime0);

    if (logger.logRawNmea2000(
      monotonicTime0,
      msg.id, msg.data)) {
      if (verbose && counter % 10 == 0) {
        console.log("Raw NMEA 2000 packet %d logged, time=%j id=%j data=%j", 
      	            counter, monotonicTime0, msg.id, msg.data);
      }
      counter++;
    }
  }
}

var channel = null;

function startNmea2000() {
  if (!channel) {
    try {
      channel = can.createRawChannel("can0", true /* ask for timestamps */);
      channel.start();
      channel.addListener("onMessage", canPacketReceived);
    } catch (e) {
      console.log("Failed to start NMEA2000");
      console.log(e);
      channel = null;
    }
  }
  return !!channel;
}

// HACK to reboot we hit SPI bug
module.exports.detectSPIBug = function(callback) {
  var timer = setInterval(function() {
    exec("ps -A -o '%C/%c' | grep kworker | sort -n -r | head -2 | sed 's#/.*##' | awk '{s+=$1} END {print s < 60}'",
         function(error, stdout, stderr) {
            if (stdout.trim() != "1") {
               clearInterval(timer);
               callback();
            }
         }
    );
  }
  , 10 * 1000);
};

module.exports.startNmea2000 = startNmea2000;
module.exports.startRawLogging = function() { rawPacketLoggingEnabled = true; };
module.exports.stopRawLogging = function() { rawPacketLoggingEnabled = false; };
module.exports.setRawLogging = function(val) { rawPacketLoggingEnabled = !!val; }; 

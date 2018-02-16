var buffer = require('buffer');
var can = require('socketcan');
var anemonode = require('../build/Release/anemonode');
var exec = require('child_process').exec;
var logger = require('./logger.js');
var boxId = require('./boxId.js');
var version = require('../version.js');
var fs = require('fs');
var pgntable = require('./pgntable.js');
var utils = require('./utils.js');

var n2kConfigFile = '/home/anemobox/n2k.config';

var nmea2000;
var nmea2000Source;
var rawPacketLoggingEnabled = false;

function configOrDefault(obj, key1, key2, def) {
  if (!obj || !obj[key1] || obj[key1][key2] === undefined) {
    return def;
  }

  return obj[key1][key2];
}

function instantiateNmea2000(cfg) {
  boxId.getAnemoId(function(boxid) {
    fs.readFile(n2kConfigFile, function(err, data) {
      if (err) {
        console.warn("Can't read " + n2kConfigFile);
        cfg.n2k = {};
      } else {
        cfg.n2k = JSON.parse(data);
      }
      instantiateNmea2000Real(boxid, cfg);
    });
  });
}

var sendEnabled = true;

function updateFromConfig(cfg) {
  sendEnabled = utils.getOrDefault(
    cfg, "outputNmea2000", true);
  setSendWindState(sendEnabled);
}

function instantiateNmea2000Real(boxid, fullCfg) {
  var cfg = fullCfg.n2k;

  updateFromConfig(fullCfg);

  var virtDevBits = 2;
  var maxNumVirtualDevices = 1 << virtDevBits;
  var baseSerialNumber =
    (parseInt(boxid, 16) & ((1 << 21 + 1 - virtDevBits) - 1)) << virtDevBits;

  var serials = [ boxid + '-0', boxid + '-1' ];
  nmea2000 = new anemonode.NMEA2000([
    {
      // product:
        serialCode: serials[0],
        productCode: 140,
        model:"Anemobox logger",
        softwareVersion: '' + version.string,
        modelVersion: 'Anemobox v1',
        loadEquivalency : 2,
        nmea2000Version: 2101,
        certificationLevel: 0xff,
      // device:
        uniqueNumber: baseSerialNumber + 0,
        manufacturerCode: 2040,
        deviceFunction: 190, // Navigation management
        deviceClass: 60,  // Navigation
        address: configOrDefault(cfg, serials[0], 'address', 10)
    }, {
      // product:
        serialCode: serials[1],
        productCode: 141,
        model:"Anemobox internal GPS",
        softwareVersion: '' + version.string,
        modelVersion: 'Anemobox v1',
        loadEquivalency : 1,
        nmea2000Version: 2101,
        certificationLevel: 0xff,
      // device:
        uniqueNumber: baseSerialNumber + 0,
        manufacturerCode: 2040,
        deviceFunction: 145, // GNSS
        deviceClass: 60,  // Navigation
        transmitPgn: [ pgntable.gnssPositionData ],
        address: configOrDefault(cfg, serials[1], 'address', 11)
    }]);

  nmea2000.onDeviceConfigChange(function() {
    var cfg = nmea2000.getDeviceConfig();
    fs.writeFile(n2kConfigFile,
                 JSON.stringify(cfg),
                 function(err) {
                   if (err) {
                     console.warn('Failed to save: ' + file);
                     console.warn(err);
                   }
                 });
  });
  nmea2000.setDeviceConfig(cfg);

  nmea2000Source = new anemonode.Nmea2000Source(nmea2000);

  nmea2000.setSendCanFrame(function(id, data) {
    if (channel && sendEnabled) {
      var msg = { id: id, data: data, ext: true };
      var r = channel.send(msg);
      return r > 0;
    } else {
      // NMEA2000 is disabled, we have no bus anyway.
      // Let's pretend it succeeded, so that tNMEA2000 class does not
      // retry to send.
      return true;
    }
  });
  
  nmea2000.open();
  setInterval(function() { 
    nmea2000.parseMessages(); 
  }, 200);
}


function canPacketReceived(msg) {
  logRawPacket(msg);
  if (nmea2000) {
    nmea2000.pushCanFrame(msg);
  }
}

// Debugging variables
var verbose = false;
var counter = 0;

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

function startNmea2000(cfg) {
  if (!channel) {
    try {
      channel = can.createRawChannel("can0", true /* ask for timestamps */);
      channel.start();
      channel.addListener("onMessage", canPacketReceived);
      instantiateNmea2000(cfg);
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


var lastSent = {
  twa: 0,
  twdir: 0,
  aw: 0
};


var sidMap = { };

function nextSid(key) {
  var sid = sidMap[key] || 0;
  sidMap[key] = (sid + 1) % 250; // <-- Good value?
  return sid;
}

var anemomindEstimatorSource = 'Anemomind estimator';
var trueWindFields = [ 'twa', 'tws', 'twdir' ];

function tryGetIfFresh(fields, sourceName, timestamp) {
  var channels = anemonode.dispatcher.allSources();
  
  // No value must be older than zis:
  var threshold = 30;
  
  // Here we accumulate the return value
  var dst = {};
  
  for (var i = 0; i < fields.length; i++) {
    var f = fields[i];
    var sourcesAndData = channels[f];
    if (!sourcesAndData) {
      return null;
    }
    var dispatchData = sourcesAndData[sourceName];
    if (!dispatchData) {
      return null;
    }
    
    var t = dispatchData.time();
    var age = timestamp - t;
    if (threshold <= age) {
      return null;
    }

    // So far, so good. Put it in the result map.
    dst[f] = [dispatchData.value(), dispatchData.unit];
  }
  return dst;
}

function trySendWind() {
  if (!nmea2000Source) {
    return; // In case we activate this code *before* having
            // started the nmea.
  }

  var now = anemonode.currentTime();
  var minResendTime = 100; // ms
  var packetsToSend = [];
  var source = anemomindEstimatorSource;
  var sources = anemonode.dispatcher.allSources();
  
  // The wind reference codes for the NMEA2000 WindData sentence
  var windAngleRefs = {
    "twa": 3,  // True boat referenced
    "twdir": 0 // True ground referenced to north
  };
  
  // Collect the packets for the different kinds of 
  // wind angle references.
  for (var wa in windAngleRefs) {
    if ((now - lastSent[wa]) > minResendTime) {
      var data2send = tryGetIfFresh([wa, "tws"], source, now);
      if (data2send) {
        packetsToSend.push({
          deviceIndex: 0,
          pgn: pgntable.windData,
          sid: nextSid(wa),
          windSpeed: data2send.tws,
          windAngle: data2send[wa],
          reference: windAngleRefs[wa]
        });
        lastSent[wa] = now;
      }
    }
  }
  
  // Send the packets, if any.
  if (0 < packetsToSend.length) {
    try {
      nmea2000Source.send(packetsToSend);
    } catch(e) {
      console.warn("Failed to send wind packets: %j", packetsToSend);
      console.warn("because");
      console.warn(e);
    }
  }
}


// Either it is null, meaning we are not sending
// anything, or it is a map, meaning we are sending.
var subscriptions = null;

function setSendWindState(shouldSend) {
  if ((subscriptions == null) == shouldSend) {  // <-- Only do something when state changed.
    if (shouldSend) {
      console.log("Start sending wind");
      subscriptions = {};
      trueWindFields.forEach(function(field) {
        var dispatchData = anemonode.dispatcher.values[field];
	      var code = dispatchData.subscribe(trySendWind);
	      subscriptions[field] = code;
      });
    } else {
      console.log("Stop sending wind");
      trueWindFields.forEach(function(field) {
        var subscription = subscriptions[field];
        if (subscription) {
          var dispatchData = anemonode.dispatcher.values[field];
          dispatchData.unsubscribe(subscription);
        }
      });
      subscriptions = null;
    }
  }
}

module.exports.updateFromConfig = updateFromConfig;
module.exports.startNmea2000 = startNmea2000;
module.exports.startRawLogging = function() { rawPacketLoggingEnabled = true; };
module.exports.stopRawLogging = function() { rawPacketLoggingEnabled = false; };
module.exports.setRawLogging = function(val) { rawPacketLoggingEnabled = !!val; }; 
module.exports.anemomindEstimatorSource = anemomindEstimatorSource;
module.exports.sendPackets = function(packets) {
  if (nmea2000Source) {
    try {
      // BUG: For some reason, the 'send' method
      // will ignore the error produced by Nan::ThrowError
      // if there are two packets being sent, and the error 
      // occurred for the first one. And when the send method
      // exits, no exception will be thrown. By sending the 
      // packets one by one, we avoid that. Maybe we should
      // change the API?
      
      for (var i = 0; i < packets.length; i++) {
        nmea2000Source.send([packets[i]]);
      }
    } catch (e) {
      // Don't spam
    }
  }
};


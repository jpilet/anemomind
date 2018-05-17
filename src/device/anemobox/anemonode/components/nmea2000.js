var assert = require('assert');
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
var sendEnabled = true;

// Debugging variables
var verbose = false;
var counter = 0;

var channel = null;
var minResendTime = 80; // ms
var sidMap = { };
var anemomindEstimatorSource = 'Anemomind estimator';
var trueWindFields = [ 'twa', 'tws', 'twdir' ];
var performanceFields = ['vmg', 'targetVmg'];
var courseFields = ['magHdg'];
var lastSentTimestamps = {
  twa: undefined,
  twdir: undefined,
  perf: undefined
};

var makePerformancePackets = rateLimitedPacketMaker(
  anemomindEstimatorSource,
  performanceFields, function(data2send) {
    var vmgSI = utils.taggedToSI(data2send.vmg);
    var targetVmgSI = utils.taggedToSI(data2send.targetVmg);
    var perf = vmgSI/targetVmgSI;
    return {
      deviceIndex: 0, // Which device?
      pgn: pgntable.BandGVmgPerformance,
      vmgPerformance: perf,
      sid: nextSid('performance')
    };
  },
  'perf');

var makeCoursePackets = rateLimitedPacketMaker(
  null,
  courseFields, function(data2send) {
    return {
      deviceIndex: 0,
      pgn: pgntable.BandGVmgPerformance,
      course: data2send.magHdg,
      sid: nextSid('performance')
    };
  },
  'course');

assert(makeCoursePackets);
assert(makePerformancePackets);

// Either it is null, meaning we are not sending
// anything, or it is a map, meaning we are sending.
var subscriptions = null;
var fieldSubscriptions = [
  {fields: trueWindFields, makePackets: makeWindPackets},
  {fields: performanceFields, makePackets: makePerformancePackets},
  {fields: courseFields, makePackets: makeCoursePackets}
];


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


function updateFromConfig(cfg) {
  sendEnabled = utils.getOrDefault(
    cfg, "outputNmea2000", true);
  setSendState(sendEnabled);
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
    /* Even if NMEA2000 output is disabled, we need to let through the packets
     * that asks other node for their NMEA2000 name (the 64 bit "address"),
     * otherwise the source will remain NMEA2000/0. If we let through packets
     * of size 3, whenever the nmea2000 system will receive data from a device
     * with an unknown name, it will send a request to that device for its
     * name. The request fits in 3 bytes. We let the request through, the
     * device sends its name, and we pick it up. Then, next time the device
     * send data, we will know its name and will tag it with the correct
     * "NMEA2000/xxxxxxxx" source string. */
    if (channel && (sendEnabled || data.length == 3)) {
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

function nextSid(key) {
  var sid = sidMap[key] || 0;
  sidMap[key] = (sid + 1) % 250; // <-- Good value?
  return sid;
}

function tryGetIfFresh(fields, sourceName, lastTimestamp, currentTimestamp) {
  var channels = anemonode.dispatcher.allSources();
  
  // No value must be older than zis:
  var threshold = 30;
  
  // Here we accumulate the return value
  var dst = {};

  // Sending too often?
  if (currentTimestamp - lastTimestamp < minResendTime) {
    return null;
  }
  
  for (var i = 0; i < fields.length; i++) {
    var f = fields[i];
    var sourcesAndData = channels[f];
    if (!sourcesAndData) {
      return null;
    }
    
    var dispatchData = sourceName? 
    	sourcesAndData[sourceName] : anemonode.dispatcher.values[f];
	
    if (!dispatchData) {
      return null;
    }
    
    // Nothing new to send?
    if (dispatchData.time() < lastTimestamp) {
      return null;
    }

    // So far, so good. Put it in the result map.
    dst[f] = [dispatchData.value(), dispatchData.unit];
  }
  return dst;
}

function makeWindPackets() {
  var now = anemonode.currentTime();
  var packetsToSend = [];
  var source = anemomindEstimatorSource;
  
  // The wind reference codes for the NMEA2000 WindData sentence
  var windAngleRefs = {
    "twa": 3,  // True boat referenced
    "twdir": 0 // True ground referenced to north
  };
  
  // Collect the packets for the different kinds of 
  // wind angle references.
  for (var wa in windAngleRefs) {
    var lastSent = lastSentTimestamps[wa] || (now - 1000);
    var data2send = tryGetIfFresh([wa, "tws"], source, lastSent, now);
    if (data2send) {
      packetsToSend.push({
        deviceIndex: 0,
        pgn: pgntable.windData,
        sid: nextSid(wa),
        windSpeed: data2send.tws,
        windAngle: data2send[wa],
        reference: windAngleRefs[wa]
      });
      lastSentTimestamps[wa] = now;
    }
  }
  return packetsToSend;
}

function rateLimitedPacketMaker(source, fields, packetMakerFunction, type) {
  return function() {
    var now = anemonode.currentTime();
    var packets = [];
    var lastSent = lastSentTimestamps[type] || (now - 1000);
    var data2send = tryGetIfFresh(fields, null, lastSent);
    if (data2send) {
      packets.push(packetMakerFunction(data2send));
      lastSentTimestamps[type] = now;
    }
    return packets;
  };
}


function subscribeForFields(fields, callback) {
  fields.forEach(function(field) {
    var dispatchData = anemonode.dispatcher.values[field];
    var code = dispatchData.subscribe(callback);
    if (field in subscriptions) {
	    subscriptions[field].push(code);
    } else {
      subscriptions[field] = [code];
    }
  });
}

function wrapSendCallback(makePackets) {
  return function() {
    if (!nmea2000Source) {
      return; // In case we activate this code *before* having
              // started the nmea.
    }
    var packetsToSend = makePackets();

    // Send the packets, if any.
    if (packetsToSend instanceof Array && 0 < packetsToSend.length) {
      try {
        nmea2000Source.send(packetsToSend);
      } catch(e) {
        console.warn("Failed to send packets: %j", packetsToSend);
        console.warn("because");
        console.warn(e);
      }
    }
  };
}

function setSendState(shouldSend) {
  if ((subscriptions == null) == shouldSend) {  // <-- Only do something when state changed.
    if (shouldSend) {
      console.log("Start sending N2k data");
      subscriptions = {};
      fieldSubscriptions.forEach(function(fieldSub) {
        subscribeForFields(
          fieldSub.fields, 
          wrapSendCallback(fieldSub.makePackets));
      });
    } else {
      console.log("Stop sending N2k data");
      for (var fieldKey in subscriptions) {
        var dispatchData = anemonode.dispatcher.values[fieldKey];
        subscriptions[fieldKey].forEach(function(subscription) {
          dispatchData.unsubscribe(subscription);
        });
      }
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
      nmea2000Source.send(packets);
    } catch (e) {
      // Don't spam
    }
  }
};

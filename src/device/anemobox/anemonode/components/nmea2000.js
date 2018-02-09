var buffer = require('buffer');
var can = require('socketcan');
var anemonode = require('../build/Release/anemonode');
var exec = require('child_process').exec;
var logger = require('./logger.js');
var boxId = require('./boxId.js');
var version = require('../version.js');
var fs = require('fs');
var pgntable = require('./pgntable.js');

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

function instantiateNmea2000() {
  boxId.getAnemoId(function(boxid) {
    fs.readFile(n2kConfigFile, function(err, data) {
      var cfg = { };
      if (err) {
        console.warn("Can't read " + n2kConfigFile);
      } else {
        cfg = JSON.parse(data);
      }
      instantiateNmea2000Real(boxid, cfg);
    });
  });
}

function instantiateNmea2000Real(boxid, cfg) {
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
        address: configOrDefault(cfg, serials[0], 'address', 42)
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
        address: configOrDefault(cfg, serials[1], 'address', 43)
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
    if (channel) {
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
  setInterval(function() { nmea2000.parseMessages(); }, 200);
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

var channel = null;

function startNmea2000() {
  if (!channel) {
    try {
      channel = can.createRawChannel("can0", true /* ask for timestamps */);
      channel.start();
      channel.addListener("onMessage", canPacketReceived);
      instantiateNmea2000();
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


var subscriptions = {};
var lastSent = {
  twa: 0,
  twdir: 0,
  aw: 0
};

function isFresh(time, field) {
  var threshold = 80;
 
  var values = anemonode.dispatcher.values[field];
  return (values.length > 0
          && Math.abs(values.time(0) - time) < threshold);
}

function getIfFresh(value, now) {
  var threshold = 80;
  if (value && ((now - value.time()) < threshold)) {
    return value.value();
  }
}

var sid = { };

function nextSid(key) {
  var sid = sid[key] || 0;
  sid[key] = sid + 1;
  return sid;
}

function trySendWind() {
  var now = anemonode.currentTime();
  var minResendTime = 100; // ms
  var packetsToSend = [];
  var sources = anemonode.dispatcher.allSources();
  var source = 'Anemomind estimator';
  if (!(source in sources)) {
    return;
  }

  if ((now - lastSent.twa) > minResendTime) {
    var twa = getIfFresh(sources.twa[source], now);
    var tws = getIfFresh(sources.tws[source], now);

    if (twa != undefined && tws != undefined) {
      packetsToSend.push({
        deviceIndex: 0,
        pgn: pgntable.windData,
        sid: nextSid('twa'),
        windSpeed: [ tws, anemonode.dispatcher.values.tws.unit ],
        windAngle: [ twa, anemonode.dispatcher.values.twa.unit ],
        reference: 3 // True (boat referenced)
      });
      lastSent.twa = now;
    }
  }

  if ((now - lastSent.twdir) > minResendTime) {
    var twdir = getIfFresh(sources.twdir[source], now);
    var tws = getIfFresh(sources.tws[source], now);

    if (twdir != undefined && tws != undefined) {
      packetsToSend.push({
        deviceIndex: 0,
        sid: nextSid('twdir'),
        pgn: pgntable.windData,
        windSpeed: [ tws, anemonode.dispatcher.values.tws.unit ],
        windAngle: [ twa, anemonode.dispatcher.values.twa.unit ],
        reference: 0 // True (ground referenced to North)
      });
      lastSent.twa = now;
    }
  }
}

function startSendingWindPackets() {
  var fields = [ 'twa', 'tws', 'twdir' ];

  for (var i in fields) {
    var field = fields[i];
    subscriptions[field] = anemonode.dispatcher.values[field].subscribe(trySendWind);
  }
}

module.exports.startNmea2000 = startNmea2000;
module.exports.startRawLogging = function() { rawPacketLoggingEnabled = true; };
module.exports.stopRawLogging = function() { rawPacketLoggingEnabled = false; };
module.exports.setRawLogging = function(val) { rawPacketLoggingEnabled = !!val; }; 
module.exports.sendPackets = function(packets) {
  if (nmea2000Source) {
    nmea2000Source.send(packets);
  }
};


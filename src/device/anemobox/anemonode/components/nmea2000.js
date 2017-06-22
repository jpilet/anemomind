// Emulate candump -L can0
var anemonode = require('../build/Release/anemonode');
var logger = require('./logger.js');
var nmea2000Source = new anemonode.Nmea2000Source();
var exec = require('child_process').exec;
var infrequent = require('./infrequent.js');

var racc = infrequent.makeAcceptor(1000);

function logRawPacket(jsocket, msg) {
  var l = logger.getLogger();
  if (l) {

  var systemTime0 = 1000*msg.ts_sec + 0.001*msg.ts_usec;
  var systemTime1 = new Date().getTime();
  var monotonicTime1 = anemonode.currentTime();
  
  // monotonicTime1 - monotonicTime0 = systemTime1 - systemTime0
  var delay = systemTime1 - systemTime0;
  monotonicTime0 = monotonicTime1 - delay;
    
    if (racc()) {
      var ds = delay + '';
      console.log("RAW DATA: %j", msg);
    }
    l.logRawNmea2000(
	monotonicTime0,
	msg.id, msg.data);
  }

  
  // This will, hopefully, trigger a packet to be
  // delivered, inside "nmea2000.js".
  jsocket.fetch();
}

var jacc = infrequent.makeAcceptor(1000);

function handlePacket(data, timestamp, srcName, pgn, priority, dstAddr, srcAddr) {
  nmea2000Source.process(data, timestamp, srcName, pgn, srcAddr);

  if (jacc()) {
    var str = "t:" + timestamp + " s:" + srcName + " pgn:" + pgn + " d:" + dstAddr;
    for (var i = 0; i < data.length; ++i) {
      str += " " + data[i].toString(16);
    }
    console.log(str);
  }
}


function getMessages(cb) {
  var rawcan = require('rawcan');
  var can = rawcan.createSocket('can0');
  can.on('error', function(err) { 
    cb(err);
  });
  can.on('message', function(msg) {
    cb(null, msg);
  });
}

function start() {
  var j1939 = require('j1939socket').j1939;
  var jsocket = new j1939.J1939Socket("can0");
  jsocket.open(handlePacket);
  getMessages(function(err, msg) {
     logRawPacket(jsocket, msg);
  });
}

function makeError(msg) {
  return "ERROR " + msg;
}

function getError(x) {
  if (5 < x.length && x.slice(0, 5) == "ERROR") {
    return x.slice(6);
  }
  return null;
}

function isNumber(x) {
  return typeof x == "number";
}

function serializeMessage(msg) {
  if (!(isNumber(msg.ts_sec) && 
  	isNumber(msg.ts_usec) && isNumber(msg.id))) {
    return makeError("Bad message format");;
  }
  if (!(msg.data instanceof Buffer)) {
    return makeError("data is not a buffer");
  }
  var dst = new Buffer(8 + 8 + 8 + msg.data.length);
  dst.writeDoubleBE(msg.ts_sec, 0);
  dst.writeDoubleBE(msg.ts_usec, 8);
  dst.writeDoubleBE(msg.id, 16);
  msg.data.copy(dst, 24);
  return dst.toString('hex');
}

function deserializeMessage(src0) {
  console.log("It is ");
  if (!(typeof src0 == "string")) {
    return {error: "Not a string"};
  }
  var err = getError(src0);
  if (err) {
    return {error: err};
  }
  var src = new Buffer(src0, 'hex');
  if (src.length < 24) {
    return {error: "Too short message"};
  }
  return {
    ts_sec: src.readDoubleBE(0),
    ts_usec: src.readDoubleBE(8),
    id: src.readDoubleBE(16),
    data: src.slice(24)
  };
}

function startCanSource() {
  getMessages(function(err, msg) {
    if (err) {
      process.stdout.write(makeError("Got error in getMessages callback") + "\n");
    } else {
      process.stdout.write(serializeMessage(msg) + "\n");
    }
  });
}

// HACK to reboot we hit SPI bug
function detectSPIBug(callback) {
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

module.exports.start = start;
module.exports.detectSPIBug = detectSPIBug;

module.exports.serializeMessage = serializeMessage;
module.exports.deserializeMessage = deserializeMessage;
module.exports.getError = getError;
module.exports.startCanSource = startCanSource;





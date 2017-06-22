// Emulate candump -L can0
var anemonode = require('../build/Release/anemonode');
var logger = require('./logger.js');
var nmea2000Source = new anemonode.Nmea2000Source();
var exec = require('child_process').exec;
var infrequent = require('./infrequent.js');

var racc = infrequent.makeAcceptor();

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
      console.log("Type of delay: %j", typeof delay);
      console.log("Delay: %s", ds);
      console.log("Logged %j sentences up to now", counter);
      console.log("Last sentence at %j", monotonicTime0);
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


function start() {
  var j1939 = require('j1939socket').j1939;
  var jsocket = new j1939.J1939Socket("can0");
  jsocket.open(handlePacket);
  var rawcan = require('rawcan');
  var can = rawcan.createSocket('can0');
  can.on('error', function(err) { console.log('socket error: ' + err); });
  can.on('message', function(msg) {
    logRawPacket(jsocket, msg);
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

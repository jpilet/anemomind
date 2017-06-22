// Emulate candump -L can0
var anemonode = require('../build/Release/anemonode');
var logger = require('./logger.js');
var nmea2000Source = new anemonode.Nmea2000Source();
var exec = require('child_process').exec;
var infrequent = require('./infrequent.js');
var canutils = require('./canutils.js');

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

function start() {
  var j1939 = require('j1939socket').j1939;
  var jsocket = new j1939.J1939Socket("can0");
  jsocket.open(handlePacket);
  canutils.getMessages(function(err, msg) {
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










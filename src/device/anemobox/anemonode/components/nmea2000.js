// Emulate candump -L can0
var buffer = require('buffer');
var anemonode = require('../build/Release/anemonode');
var logger = require('./logger.js');
var assert = require('assert');
var nmea2000Source = new anemonode.Nmea2000Source();
var exec = require('child_process').exec;

var counter = 0;

function logRawPacket(socket, msg) {
  var l = logger.getLogger();
  if (l) {

  var systemTime0 = 1000*msg.ts_sec + 0.001*msg.ts_usec;
  var systemTime1 = new Date().getTime();
  var monotonicTime1 = anemonode.currentTime();
  
  // monotonicTime1 - monotonicTime0 = systemTime1 - systemTime0
  var delay = systemTime1 - systemTime0;
  monotonicTime0 = monotonicTime1 - delay;
    
    counter++;
    if (counter % 100 == 0) {
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
  socket.fetch();
}

var counter2 = 0;


function handlePacket(data, timestamp, srcName, pgn, priority, dstAddr, srcAddr) {
  nmea2000Source.process(data, timestamp, srcName, pgn, srcAddr);

  if (counter2 % 100 == 0) {
    var str = "t:" + timestamp + " s:" + srcName + " pgn:" + pgn + " d:" + dstAddr;

    for (var i = 0; i < data.length; ++i) {
      str += " " + data[i].toString(16);
    }
    console.log(str);
  }
  counter2++;
}


function createAndSubscribeToChannel(can, socket) {
  console.log("Create new channel!");
  var channel = can.createRawChannel("can0", true /* ask for timestamps */);
  channel.start();
  channel.addListener("onMessage", function(msg) {logRawPacket(socket, msg);});
  return channel;
}

function Context() {
  console.log("Load the modules");
  this.j1939 = require('j1939socket').j1939;
  console.log("Create a new socket");
  this.socket = new this.j1939.J1939Socket("can0");
  console.log("Open the socket");
  this.socket.open(handlePacket);
  this.can = require('socketcan');
  console.log("Create the channel");
  this.channel = createAndSubscribeToChannel(this.can, this.socket);
}

function unload(moduleName) {
  var name = require.resolve(moduleName);
  delete require.cache[name];
}

Context.prototype.destroy = function() {
  this.channel.stop();
  unload('socketcan');
  unload('j1939socket');
}

var context = null;
function restart() {
  try {
  if (context) {
    console.log("Destroy old context");
    context.destroy();
  }
  context = new Context();
  } catch (e) {
    console.log("Failed to listen to CAN channel");
    console.log(e);
  }
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

module.exports.restart = restart;
module.exports.detectSPIBug = detectSPIBug;

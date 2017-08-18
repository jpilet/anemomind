// Emulate candump -L can0
var buffer = require('buffer');
var anemonode = require('../build/Release/anemonode');
var logger = require('./logger.js');
var assert = require('assert');

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

function unload(x) {
  var name = require.resolve(x);
  delete require.cache[name];
}

function start() {
  try {
  console.log("Load the modules");
  var j1939 = require('j1939socket').j1939;
  console.log("Create a new socket");
  var socket = new j1939.J1939Socket("can0");
  console.log("Open the socket");
  socket.open(function (data, timestamp, srcName,
                      pgn, priority, dstAddr, srcAddr) {
          console.log("t:" + timestamp + " s:" + srcName + " pgn:" + pgn
              + "src: " + srcAddr + " d:" + dstAddr + " " + data.toString('hex').toUpperCase());
   });
  var can = require('socketcan');
  var buffer = require('buffer');

  console.log("Create the channel");
  var channel = can.createRawChannel("can0", true /* ask for timestamps */);
  channel.start();

  assert(j1939);
  channel.addListener("onMessage", function(msg) {logRawPacket(socket, msg);});

  } catch (e) {
    console.log("Failed to listen to CAN channel");
    console.log(e);
  }
}

module.exports.start = start;

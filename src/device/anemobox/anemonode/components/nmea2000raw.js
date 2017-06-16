// Emulate candump -L can0
var j1939socket = require('j1939socket');
var can = require('socketcan');
var buffer = require('buffer');
var anemonode = require('../build/Release/anemonode');
var logger = require('./logger.js');

function logRawPacket(msg) {
  var systemTime0 = 1000*msg.ts_sec + 0.001*msg.ts_usec;
  var systemTime1 = new Date().getTime();
  var monotonicTime1 = anemonode.currentTime();
  
  // monotonicTime1 - monotonicTime0 = systemTime1 - systemTime0
  monotonicTime0 = monotonicTime1 - (systemTime1 - systemTime0);
    
  console.log('LOG RAW PACKET at %j', monotonicTime0);

  var l = logger.getLogger();
  if (l) {
    l.logRawNmea2000(
	monotonicTime0,
	msg.id, msg.data.toString());
  }
  
  // This will, hopefully, trigger a packet to be
  // delivered, inside "nmea2000.js".
  j1939socket.fetch();
}

function start() { 
  try {
    var channel = can.createRawChannel("can0", true /* ask for timestamps */);
    channel.start();
    channel.addListener("onMessage", logRawPacket);
  } catch (e) {
    console.log("Failed to listen to CAN channel");
    console.log(e);
  }
}

module.exports.start = start;

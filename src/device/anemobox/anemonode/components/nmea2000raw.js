// Emulate candump -L can0
var j1939socket = require('j1939socket');
var can = require('socketcan');
var buffer = require('buffer');
var anemonode = require('../build/Release/anemonode');
var channel = can.createRawChannel("can0", true /* ask for timestamps */);
channel.start();

function logRawPacket(msg) {
  var systemTime0 = 1000*msg.ts_sec + 0.001*msg.ts_usec;
  var systemTime1 = new Date().getTime();
  var monotonicTime1 = anemonode.currentTime();
  
  // monotonicTime1 - monotonicTime0 = systemTime1 - systemTime0
  
  monotonicTime0 = monotonicTime1 - (systemTime1 - systemTime0);

  anemonode.logRawNmea2000(
    monotonicTime0, 
    msg.id, msg.data.toString());
  
  // This will, hopefully, trigger a packet to be
  // delivered, inside "nmea2000.js".
  j1939socket.fetch();
}

channel.addListener("onMessage", logRawPacket);


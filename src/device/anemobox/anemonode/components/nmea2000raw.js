// Emulate candump -L can0
var j1939socket = require('j1939socket');
var can = require('socketcan');
var buffer = require('buffer');
var anemonode = require('anemonode');

var channel = can.createRawChannel("can0", true /* ask for timestamps */);
channel.start();

function dumpPacket(msg) {
  var tsMilliseconds = 1000*msg.ts_sec + 0.001*msg.ts_usec;
  

    toHex(msg.id).toUpperCase() + '#' + msg.data.toString('hex').toUpperCase());
  j1939socket.fetch();
}

channel.addListener("onMessage", logRawPacket);


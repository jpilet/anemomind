// Data source: NMEA0183

var SerialPort = require("serialport").SerialPort
var fs = require('fs');

var nmea0183Port;

function init(nmea0183PortPath) {
  // Let flow control make the port work.
  fs.writeFile('/sys/kernel/debug/gpio_debug/gpio129/current_pinmux', 'mode0', function() {});
  fs.writeFile('/sys/kernel/debug/gpio_debug/gpio129/current_value', 'high', function() {});

  var port = new SerialPort(nmea0183PortPath, {
    baudrate: 4800
  }, false); // this is the openImmediately flag [default is true]


  port.open(function (error) {
    if ( error ) {
      console.log('failed to open: '+error);
    } else {
      console.log('Listening to NMEA0183 port ' + nmea0183PortPath);
      nmea0183Port = port;
      port.on('data', function(data) {
        //console.log('Received: ' + data);
      });
    }
  });
}

function emitNmea0183Sentence (sentence) {
  if (nmea0183Port) {
    console.log('sending: ' + sentence);
    nmea0183Port.write(sentence);
  }
}

init('/dev/ttyMFD1');


var assert = require('assert');

function nmeaPacket(data) {
  var sum = 0;
  for (var i = 0; i < data.length; ++i) {
    sum ^= data.charCodeAt(i);
  }
  var hexsum = sum.toString(16);
  if (hexsum.length == 1) {
    hexsum = "0" + hexsum;
  }

  return "$" + data + "*" + hexsum + "\r\n";
}

function tacktickCustomPage(pageno, header, footer) {
  assert(pageno >= 1);
  assert(pageno <= 4);
  return nmeaPacket("PTAK,FFP" + pageno + "," + header + "," + footer);
}

function tacktickPageData(pageno, data) {
  return nmeaPacket("PTAK,FFD" + pageno + "," + data);
}

function tacktickWindCorrect(twaDeltaDeg, twsDeltaKnots) {
  return nmeaPacket("PTAK,TWC,"
   + twsDeltaKnots.toFixed(1) + ","
   + twaDeltaDeg.toFixed(0));
}

setInterval(function() {
  emitNmea0183Sentence(tacktickCustomPage(1, "ANEMO", "MIND"));
}, 20 * 1000);

var i = 0;
setInterval(function() {
  ++i;
  if (i > 100) i = 0;
  emitNmea0183Sentence(tacktickPageData(1, i));
}, 500);

function randRange(min, max) {
  return (max - min) * Math.random() + min;
}

setInterval(function() {
  var deltaAngle = randRange(-30, 30);
  var deltaSpeed = randRange(-10, 10);
  //emitNmea0183Sentence(tacktickWindCorrect(deltaAngle, deltaSpeed));
}, 3000);


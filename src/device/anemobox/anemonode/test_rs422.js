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
        console.log('Received: ' + data);
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

var counter = 0;
setInterval(function() {
  emitNmea0183Sentence('Hello ' + ++counter);
  }, 1000);


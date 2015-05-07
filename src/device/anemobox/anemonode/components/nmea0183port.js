// Data source: NMEA0183


var anemonode = require('./build/Release/anemonode');
var SerialPort = require("serialport").SerialPort

var nmea0183Port;

function init(nmea0183PortPath) {
  var port = new SerialPort(nmea0183PortPath, {
    baudrate: 4800
  }, false); // this is the openImmediately flag [default is true]

  var nmeaPortSource = new anemonode.Nmea0183Source(
      "NMEA0183: " + nmea0183PortPath);

  port.open(function (error) {
    if ( error ) {
      console.log('failed to open: '+error);
    } else {
      console.log('Listening to NMEA0183 port ' + nmea0183PortPath);
      nmea0183Port = port;
      port.on('data', function(data) {
        nmeaPortSource.process(data);
        // TODO: log raw NMEA.
      });
    }
  });
}

function emitNmea0183Sentence (sentence) {
  if (nmea0183Port) {
    nmea0183Port.write(sentence);
  }
}

module.export.init = init;
module.export.emitNmea0183Sentence = emitNmea0183Sentence;


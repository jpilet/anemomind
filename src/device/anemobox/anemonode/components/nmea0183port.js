// Data source: NMEA0183

var anemonode = require('../build/Release/anemonode');
var SerialPort = require("serialport").SerialPort
var fs = require('fs');
var config = require('./config');

var nmea0183Port;

function init(nmea0183PortPath, dataCb) {
  // Let flow control make the port work.
  fs.writeFile('/sys/kernel/debug/gpio_debug/gpio129/current_pinmux',
               'mode0', function() {});
  fs.writeFile('/sys/kernel/debug/gpio_debug/gpio129/current_value',
               'high', function() {});


  config.get(function(err, config) {
    var port = new SerialPort(nmea0183PortPath, {
      baudrate: config.nmea0183Speed
    }, false); // this is the openImmediately flag [default is true]

    var nmeaPortSource = new anemonode.Nmea0183Source(
        "NMEA0183: " + nmea0183PortPath);

    port.open(function (error) {
      if ( error ) {
        console.log('failed to open: '+error);
      } else {
        console.log('Listening to NMEA0183 port ' + nmea0183PortPath
                    + ' at ' + config.nmea0183Speed + ' bauds');
        nmea0183Port = port;
        port.on('data', function(data) {
          nmeaPortSource.process(data);
          if (dataCb) {
            dataCb(data);
          }
        });
      }
    });
  });
}

function emitNmea0183Sentence (sentence) {
  if (nmea0183Port) {
    nmea0183Port.write(sentence);
  }
}

module.exports.init = init;
module.exports.emitNmea0183Sentence = emitNmea0183Sentence;


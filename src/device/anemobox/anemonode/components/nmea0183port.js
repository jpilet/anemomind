// Data source: NMEA0183

var anemonode = require('../build/Release/anemonode');
var SerialPort = require("serialport").SerialPort
var fs = require('fs');
var config = require('./config');

var nmea0183Port;
var initializedSpeed;
var source = '';

function sourceName() { return source; }

config.events.on('change', function() {
  config.get(function(err, cfg) {
    if (!nmea0183Port || initializedSpeed != cfg.nmea0183Speed) {
      module.exports.reset();
    }
  });
});

function init(nmea0183PortPath, dataCb) {

  require('./pinconfig').activateNmea0183();

  config.get(function(err, config) {
    initializedSpeed = config.nmea0183Speed;

    var port = new SerialPort(nmea0183PortPath, {
      baudrate: config.nmea0183Speed,
      /* man 3 termios says:
         TIME  specifies  the limit for a timer in tenths of a second.  Once
         an initial byte of input becomes available, the timer is restarted
         after each further byte is received.  read(2) returns when any of
         the following conditions is met:

            *  MIN bytes have been received.
            *  The interbyte timer expires.
            *  The number of bytes requested by read(2) has been received.
      */
      vtime: 1, // introduce at most 0.1 sec of delay
      vmin: 16, // try to read 16 bytes in a row
      bufferSize: 16, // this is the number of bytes requested by read(2)
      autoOpen: false
    });

    source = "NMEA0183: " + nmea0183PortPath;
    var nmeaPortSource = new anemonode.Nmea0183Source(source);

    module.exports.reset = function() {
      // prevent emitNmea0183Sentence to send anything, we're closing.
      nmea0183Port = undefined;

      // ignore resets while we are resetting.
      module.exports.reset = function() { };

      port.close(function() { init(nmea0183PortPath, dataCb); });
    };

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
        port.on('error', function(err) {
          console.warn(err);
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
module.exports.sourceName = sourceName;
module.exports.reset = function() {};


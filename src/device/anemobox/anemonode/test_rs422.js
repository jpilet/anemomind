// Data source: NMEA0183

var SerialPort = require("serialport")

process.env.ANEMOBOX_CONFIG_PATH = '/home/anemobox/';

var config = require('./components/config');

var nmea0183Port;

function init(nmea0183PortPath) {
  config.get(function(err, config) {
    if (err) {
      console.warn(err);
    }
    require('./components/pinconfig').activateNmea0183();

    var port = new SerialPort(nmea0183PortPath, {
      baudrate: parseInt(config.nmea0183Speed),
      vtime: 1, // introduce at most 0.1 sec of delay
      vmin: 80, // try to read 80 bytes in a row
      bufferSize: 80, // this is the number of bytes requested by read(3)
      autoOpen: false
    });


    port.open(function (error) {
      if ( error ) {
        console.log('failed to open: '+error);
      } else {
        console.log('Listening to NMEA0183 port ' + nmea0183PortPath
                    + ' speed: ' + config.nmea0183Speed);
        nmea0183Port = port;
        port.on('data', function(data) {
          process.stdout.write(data.toString('ascii'));
        });
      }
    });
  });
}

function emitNmea0183Sentence (sentence) {
  if (nmea0183Port) {
    console.log('[sending: ' + sentence + ']');
    nmea0183Port.write(sentence);
  }
}

init('/dev/ttyMFD1');

var counter = 0;
setInterval(function() {
  emitNmea0183Sentence('Hello ' + ++counter);
  }, 1000);


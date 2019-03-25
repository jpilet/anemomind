// Data source: NMEA0183

var SerialPort = require("serialport")

process.env.ANEMOBOX_CONFIG_PATH = '/home/anemobox/';

var config = require('./components/config');

var nmea = require('./components/nmea0183perf');

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
    console.log('  sending: ' + sentence);
    nmea0183Port.write(sentence);
  }
}

init('/dev/ttyMFD1');

var sentences = '\n$IIRMB,A,0.11,R,,YVOI,,,,,015.92,242,02.4,V,A*73\n$IIRMC,164510,A,4629.736,N,00639.790,E,03.6,197,200808,,,A*4F\n$IIVHW,,,195,M,03.4,N,,*6E\n$IIVWR,028,L,07.7,N,,,,*75 \n$IIDPT,062.0,-1.0,*44\n$IIGLL,4629.736,N,00639.790,E,164510,A,A*5E\n$IIHDG,195,,,,*5A\n$IIMTW,+21.0,C*3B\n$IIMWV,332,R,07.7,N,A*11\n$IIMWV,312,T,04.8,N,A*19';

var start = new Date().getTime();
var counter = 0;
setInterval(function() {
  //emitNmea0183Sentence('Hello ' + ++counter);
  //emitNmea0183Sentence(sentences);
 
  var dt = (new Date()).getTime() - start;
  
  var perf = (100 + Math.sin(dt / 9) * 30);
  var targetVmg = (5 + Math.sin(7 + dt / 9) * 5);

  //perf = (Math.round(dt / 1000) % 2) == 1 ? 0 : 100;
  perf = 0;
  targetVmg = 0;
  emitNmea0183Sentence(nmea.nkeDynamic('PV', 1, 4,
                                  Math.round(10 * perf),
                                  'VMG Perf', '%'));
  emitNmea0183Sentence(nmea.nkeDynamic('TV', 2, 0,
                                  Math.round(100 * targetVmg),
                                  'VMG Target', 'Nds'));
                                  
  }, 500);


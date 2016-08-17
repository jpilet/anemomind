// Data source: NMEA0183

var SerialPort = require("serialport").SerialPort
var fs = require('fs');

var nmea0183Port;

var receivedData = false;
var receivedValidPong = false;
var lastSentPing;

function checkDone() {
  if (receivedData && receivedValidPong) {
    console.log('NMEA0183: OK');
    process.exit(0);
  }
}

function startFailTimeout() {
  setInterval(function() {
    console.log('NMEA0183 port: '
      + (receivedData ? 'data received' : 'can\'t receive data')
      + ', '
      + (receivedValidPong ? 'data sent' : ' can\'t send data'));
    checkDone();
    //process.exit(1);
  }, 3000);
}

function startPing() {
  var counter = 0;
  setInterval(function() {
    ++counter;
    emitNmea0183Sentence('Hello ' + counter);
    lastSentPing = counter;
  }, 1000);
}

function init(nmea0183PortPath) {
  require('./components/pinconfig').activateNmea0183();

  var port = new SerialPort(nmea0183PortPath, {
    baudrate: 4800
  }, false); // this is the openImmediately flag [default is true]


  port.open(function (error) {
    if ( error ) {
      console.log('failed to open: '+error);
      process.exit(2);
    } else {
      startFailTimeout();
      startPing();
      nmea0183Port = port;
      port.on('data', function(dataBuffer) {
              var data = dataBuffer.toString();

              if (data.match(/Hello \d+/)) {
                receivedData = true;
              }
              var m = data.match(/pong (\d+)/);
              if (m && m.length == 2 && parseInt(m[1]) == lastSentPing) {
               receivedValidPong = true;
              }
              checkDone();
      });
    }
  });
}

function emitNmea0183Sentence (sentence) {
  if (nmea0183Port) {
    nmea0183Port.write(sentence);
  }
}

init('/dev/ttyMFD1');


// Data source: NMEA0183

var anemonode = require('../build/Release/anemonode');
var exec = require('child_process').exec;
var fs = require('fs');

var nmeaPortFd;

function init(port) {
exec("stty -F " + port + " 4800",  function (error, stdout, stderr) {
  if (error) { console.log(error); }

  fs.open(port, 'r+', function(err, fd) {

    nmeaPortFd = fd;
  var buffer = new Buffer(4096);
  var nmeaPortSource = new anemonode.Nmea0183Source(
      "NMEA0183: " + port);

    setInterval(function() {
      fs.read(fd, buffer, 0, 4096, null, function(err, bytes_read, buffer) {
	 if (err) {
	   console.log(err);
	 } else {
	   //console.log('received: ' +bytes_read + ' bytes: ' +  buffer.slice(0, bytes_read));
	   nmeaPortSource.process(buffer.slice(0, bytes_read));
	 }
       });
    }, 500);
  });
});
}

function emitNmea0183Sentence (sentence) {
  if (nmeaPortFd) {
    //console.log(sentence.toString('ascii'));
    fs.write(nmeaPortFd, sentence, 0, sentence.length, function(err, written, buffer) {
});
  }
}

module.exports.init = init;
module.exports.emitNmea0183Sentence = emitNmea0183Sentence;

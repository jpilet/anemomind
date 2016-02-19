var j1939socket = require('j1939socket');
var anemonode = require('../build/Release/anemonode');
var nmea2000Source = new anemonode.Nmea2000Source();
var exec = require('child_process').exec;

function handlePacket(data, timestamp, srcName, pgn, priority, dstAddr) {
  nmea2000Source.process(data, timestamp, srcName, pgn);

  if (false) {
    var str = "t:" + timestamp + " s:" + srcName + " pgn:" + pgn + " d:" + dstAddr;

    for (var i = 0; i < data.length; ++i) {
      str += " " + data[i].toString(16);
    }
    console.log(str);
  }
}

try {
  j1939socket.open(handlePacket);
} catch (error) {
  console.log("Error, NMEA2000 disabled.");
}

// HACK to reboot we hit SPI bug
module.exports.detectSPIBug = function(callback) {
  var timer = setInterval(function() {
    exec("ps -A -o '%C/%c' | grep kworker | sort -n -r | head -2 | sed 's#/.*##' | awk '{s+=$1} END {print s < 60}'",
         function(error, stdout, stderr) {
            if (stdout.trim() != "1") {
               clearInterval(timer);
               callback();
            }
         }
    );
  }
  , 10 * 1000);
};


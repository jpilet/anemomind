var fs = require('fs');


var watchdogFd;

function startWatchdog() {
  if (watchdogFd) {
    console.log('watchdog: already started');
    return;
  }

  fs.writeFile('/sys/devices/virtual/misc/watchdog/disable', '0', function(err) {
    if (err) {
      console.log('watchdog error: can\'t enable!');
      return;
    }
    console.log('watchdog enabled');

    var dev = '/dev/watchdog';

    var pingWatchdog = function() {
      fs.write(watchdogFd, 'R', function(err, bytes) {
        if (err) {
          console.log(dev + ': ' + err);
        }
      });
    };

    var closeWatchdog = 
    fs.open(dev, 'w', function(err, fd) {
      if (err) {
        console.log(dev + ': ' + err);
      } else {
        watchdogFd = fd;
        setInterval(pingWatchdog, 30 * 1000);
        module.exports.closeWatchdog = function(done) {
          fw.write(watchdogFd, 'V', function() { if (done) done(); });
        };
      }
    });
  });
}

module.exports.startWatchdog = startWatchdog;
module.exports.closeWatchdog = function() { };

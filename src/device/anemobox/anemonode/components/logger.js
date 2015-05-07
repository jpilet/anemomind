var anemonode = require('../build/Release/anemonode');
var fs = require('fs');

function startLogging(logRoot, logInterval, cb) {
  function createAndScheduleLogger() {
    var logger = new anemonode.Logger();
    setInterval(function() {
      logger.flush(logRoot, function(path, err) {
	if (err) {
	  console.log(err);
	} else {
	  cb(path);
	}
      });
    }, logInterval);
  }

  fs.exists(logRoot, function(exists) {
    if (!exists) {
      fs.mkdir(logRoot, 0755, function() { });
    }
    createAndScheduleLogger();
  });
}

module.exports.startLogging = startLogging;

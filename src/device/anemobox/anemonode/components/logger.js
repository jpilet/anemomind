var anemonode = require('../build/Release/anemonode');
var mkdirp = require('mkdirp');

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

  mkdirp(logRoot, 0755, function(err) {
    if (err) {
      console.log('Failed to create path: ' + logRoot);
    } else {
      console.log('Logging dispatcher data to: ' + logRoot);
      createAndScheduleLogger();
    }
  });
}

module.exports.startLogging = startLogging;

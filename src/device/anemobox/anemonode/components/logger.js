var anemonode = require('../build/Release/anemonode');
var mkdirp = require('mkdirp');

var logger;
module.exports.flush = function() {};

function startLogging(logRoot, logInterval, cb) {
  function createAndScheduleLogger() {
    logger = new anemonode.Logger();

    module.exports.flush = function() {
      logger.flush(logRoot, function(path, err) {
	if (err) {
	  console.log(err);
	} else {
	  cb(path);
	}
      });
    }; 

    setInterval(module.exports.flush, logInterval);
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

function logText(stream, text) {
  if (logger) {
    logger.logText(stream, text);
  }
}

module.exports.startLogging = startLogging;
module.exports.logText = logText;

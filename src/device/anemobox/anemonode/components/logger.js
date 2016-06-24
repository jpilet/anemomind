var anemonode = require('../build/Release/anemonode');
var mkdirp = require('mkdirp');

function int64ToHexString(x) {
  var s = x.toString(16).toUpperCase();
  return "0".repeat(Math.max(0, 16 - s.length)) + s;
}

function timeToHexString(x) {
  assert(x instanceof Date);
  return int64ToHexString(x.getTime());
}

var logger;
module.exports.flush = function() {};

function startLogging(logRoot, logInterval, getCurrentTime, cb) {
  function createAndScheduleLogger() {
    logger = new anemonode.Logger();

    module.exports.flush = function() {
      var filename = logRoot + timeToHexString(getCurrentTime()) + ".log";
      logger.flush(filename, function(path, err) {
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

var anemonode = require('./build/Release/anemonode');

function startLogging(logRoot, logInterval, cb) {
  var logger = new anemonode.Logger();
  setInterval(function() {
    logger.flush(logRoot, function(path, err)) {
      if (err) {
        console.log(err);
      } else {
        cb(path);
      }
    });
  }, logInterval);
}

module.exports.startLogging = startLogging;

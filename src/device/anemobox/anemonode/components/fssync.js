var exec = require('child_process').exec;

module.exports.exec = function(cb) {
  exec('sync', cb || function() { });
});


var exec = require('child_process').exec;

module.exports.sync = function(cb) {
  exec('sync', cb || function() { });
};


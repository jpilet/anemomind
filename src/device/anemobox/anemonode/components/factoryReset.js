var exec = require('child_process').exec;

module.exports = function(cb) {
  exec('/anemonode/factory_reset.sh', cb);
};

var exec = require('child_process').exec;
var fs = require('fs');

module.exports.reboot = function() {
  exec('sync', function() {
    fs.writeFileSync('/sys/devices/virtual/misc/watchdog/disable', '0');
    exec('reboot', function() {});
  });
};

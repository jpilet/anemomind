var fs = require('fs');

function setPinOutput(pinNo, value) {
  var path = '/sys/kernel/debug/gpio_debug/gpio' + pinNo + '/';
  fs.writeFile(path + 'current_pinmux', 'mode0', function() {});
  fs.writeFile(path + 'current_direction', 'out', function() {});
  fs.writeFile(path + 'current_value', value, function() {});
}

module.exports.activateNmea0183 = function () {
  // for anemobox 2.3.1 : activate driver output
  setPinOutput(183, 'high');

  // for anemobox 2.3.1 : activate receiver output
  setPinOutput(45, 'low');

  // For anemobox v1.1x
  // Let flow control make the port work.
  setPinOutput(129, 'high');
};


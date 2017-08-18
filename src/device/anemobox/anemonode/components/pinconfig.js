var fs = require('fs');

function setPinOutput(pinNo, value) {
  var path = '/sys/kernel/debug/gpio_debug/gpio' + pinNo + '/';
  fs.writeFileSync(path + 'current_pinmux', 'mode0');
  fs.writeFileSync(path + 'current_direction', 'out');
  fs.writeFileSync(path + 'current_value', value);
}

module.exports.activateNmea0183 = function () {
  // for anemobox 2.3.1 : activate driver output
  setPinOutput(183, 'high');

  // for anemobox 2.3.1 : activate receiver output
  setPinOutput(45, 'low');

  // For anemobox v1.1x
  // Let flow control make the port work.
  setPinOutput(129, 'high');
  setInterval(function() {
    setPinOutput(129, 'high');
  }, 5000);
};


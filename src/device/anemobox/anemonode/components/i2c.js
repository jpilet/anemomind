var mraa = require('mraa');

var I2CBUS_ADAPTER=1
var i2c;
function reset() {
  i2c = new mraa.I2c(I2CBUS_ADAPTER);
};
reset();

module.exports.i2c = i2c;
module.exports.reset = reset;

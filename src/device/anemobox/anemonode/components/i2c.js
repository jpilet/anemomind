var mraa = require('mraa');

var I2CBUS_ADAPTER=1
module.exports.i2c = new mraa.I2c(I2CBUS_ADAPTER);


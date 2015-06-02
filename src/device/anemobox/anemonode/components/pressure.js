
var i2c = require('./i2c').i2c;

var initialized = false;

// Constants.
var LPS331AP_I2C_BASE_ADDR = 0x5C;

var LPS331AP_READ_ADDRESS = 0xB8;
var LPS331AP_WRITE_ADDRESS = 0xB9;
var I2C_AUTO_INCREMENT = 0x80	;

var LPS331AP_WHOAMI = 0xBB;

var LPS331AP_RES_REF_P_XL = 0;
var LPS331AP_RES_REF_P_L = 1;
var LPS331AP_RES_REF_P_H = 2;
var LPS331AP_RES_REF_T_L = 3;
var LPS331AP_RES_REF_T_H = 4;
var LPS331AP_RES_TP_RESOL = 5;
var LPS331AP_RES_CTRL_REG1 = 6;
var LPS331AP_RES_CTRL_REG2 = 7;
var LPS331AP_RES_CTRL_REG3 = 8;
var LPS331AP_RES_INT_CFG_REG = 9;
var LPS331AP_RES_THS_P_L = 10;
var LPS331AP_RES_THS_P_H = 11;

var CTRL_REG1 = 0x20;	/*	power / ODR control reg	*/
var CTRL_REG2 = 0x21;	/*	boot reg		*/
var CTRL_REG3 = 0x22;	/*	interrupt control reg	*/
var RES_CONF = 0x10;
var INT_CFG_REG = 0x23;	/*	interrupt config reg	*/
var INT_SRC_REG = 0x24;	/*	interrupt source reg	*/
var THS_P_L = 0x25;	/*	pressure threshold	*/
var THS_P_H = 0x26;	/*	pressure threshold	*/
var STATUS_REG = 0X27;	/*	status reg		*/


function init() {
  i2c.address(LPS331AP_I2C_BASE_ADDR);

  var whoAmICommand = 0xF;
  var whoAmIAnswer = 0xBB;
  var r = i2c.readReg(whoAmICommand);

  if (r == whoAmIAnswer) {
    initialized = true;
    i2c.writeReg(0xE0, CTRL_REG1);
  }
  console.log('Pressure sensor ' + (initialized ? 'found' : 'not found'));
  return initialized;
}

function pressure(numReads) {
  i2c.address(LPS331AP_I2C_BASE_ADDR);

  return average(numReads, function() {
    var PRESS_OUT_XL = 0x28;

    var press = [0,0,0];
    for (var i = 0; i < 3; ++i) {
      press[i] = i2c.readReg(PRESS_OUT_XL + i);
    }
    return ((press[2] << 16) + (press[1] << 8) + press[0]) / 4096.0;
  });
}

function temperature(numReads) {
  i2c.address(LPS331AP_I2C_BASE_ADDR);

  return average(numReads, function () {
    var TEMP_OUT_L = 0x2b;
    var TEMP_OUT_H = 0x2c;

    var buffer = new Buffer(2);
    buffer[0] = i2c.readReg(TEMP_OUT_L);
    buffer[1] = i2c.readReg(TEMP_OUT_H); 

    var t = buffer.readInt8(0);
    return t/480.0 + 42.5 
  });
}

function average(numReads, read) {
  if (numReads) {
    var sum = 0;
    for (var i = 0 ; i < numReads; ++i) {
      sum += read();
    }
    return sum / numReads;
  } else {
    return read();
  }
}

module.exports.init = init;
module.exports.pressure = pressure;
module.exports.temperature = temperature;

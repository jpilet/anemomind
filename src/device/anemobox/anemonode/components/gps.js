var i2c = require('./i2c').i2c;
var anemonode = require('../build/Release/anemonode');

// GPS
var CAM_M8Q_I2C_BASE_ADDR=0x42
var CAM_M8Q_REG_ADDRESS_LOW=0x00
var CAM_M8Q_REG_ADDRESS_HIGH=0xFC
var CAM_M8Q_REG_NB_BYTES_AVAIL_HIGH=0xFD
var CAM_M8Q_REG_NB_BYTES_AVAIL_LOW=0xFE
var CAM_M8Q_REG_DATA_STREAM=0xFF
var CHECK_OUT_STREAM=0xFF


function init() {
  console.log('Reading internal GPS');

  var nmeaSource = new anemonode.Nmea0183Source("Internal GPS");


  setInterval(function() {
    i2c.address(CAM_M8Q_I2C_BASE_ADDR);
    var data = [];
    do {
      var c = i2c.readReg(CAM_M8Q_REG_DATA_STREAM);
      if (c != 255) {
        data.push(c);
      } else {
        break;
      }
    } while(1);

    if (data.length > 0) {
      var buffer = new Buffer(data);
      nmeaSource.process(buffer);
    }
  }, 50);
}

module.exports.init = init;

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

var nmeaSource = new anemonode.Nmea0183Source("Internal GPS");

function readGps(dataCb) {
  var data;

  i2c.address(CAM_M8Q_I2C_BASE_ADDR);

  // The first read is 1: quickly exit if there is no data to read.
  var readSize = 1;
  do {
    var c = i2c.readBytesReg(CAM_M8Q_REG_DATA_STREAM, readSize);
    if (data) {
      data = Buffer.concat([data, c]);
      // If there is more data to read, we should read more than
      // 1 byte at a time.
      readSize = 128;
    } else {
      data = c;
    }
    if (data.length > 0 && data[data.length - 1] == 255) {
      break;
    }
  } while(1);

  if (data.length > 0 && data[0] != 255) {
    // Trim padded '255'
    for (var i = data.length - 1; data[i] == 255; --i) {
      data = data.slice(0, i + 1);
    }
    nmeaSource.process(data);
    if (dataCb) {
      dataCb(data);
    }
  }
}

module.exports.readGps = readGps;

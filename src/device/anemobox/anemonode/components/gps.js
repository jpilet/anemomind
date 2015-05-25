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


function init(dataCb) {
  console.log('Reading internal GPS');

  var nmeaSource = new anemonode.Nmea0183Source("Internal GPS");



/* does not work yet
  function format(x) {
      return JSON.stringify(x);
  }
function printHistory(field) {
  for (var i = 0; i < anemonode.dispatcher[field].length(); ++i) {
    var dispatchData = anemonode.dispatcher[field];
    console.log(anemonode.dispatcher[field].description
                + ': ' + format(dispatchData.value(i))
                + ' ' + anemonode.dispatcher[field].unit
                + ' set on: ' + dispatchData.time(i).toLocaleString());
  }
}

  anemonode.dispatcher.dateTime.subscribe(function(val) {
    var sysTime = anemonode.dispatcher.dateTime.time(0);
    var gpsTime = anemonode.dispatcher.dateTime.value(0);
    var delta = 0 + gpsTime - sysTime;
    console.log('sysTime:'+sysTime);
    console.log('gpsTime:'+val);
    if (delta > 1) {
      console.log('Calling adjtime with delta: ' + delta);
      nmeaSource.adjTime(delta);
    } else {
      console.log('not calling adjtime, delta=' + delta);
      }
  });
*/

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
      if (dataCb) {
        dataCb(buffer);
      }
    }
  }, 50);
}

module.exports.init = init;

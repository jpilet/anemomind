var calypso = require('calypso_ultrasonic');
var anemonode = require('../build/Release/anemonode');

function Watchdog(cb) {
  this.cb = cb;
  this.timeout = undefined;
}

Watchdog.prototype.next = function(t) {
  if (this.timeout) {
    clearTimeout(this.timeout);
  }
  this.timeout = setTimeout(this.cb, t);
};

function peripheralDiscovered(err, peripheral) {
  if (err) {
    console.warn(err);
    return;
  }
  console.log('found calypso ultrasonic: ', peripheral.id);

  calypso.stopScanning();

  // In case of timeout, we close the connection and start scanning again.
  var watchdog = new Watchdog(function() {
    console.warn('No more data from calypso ' + peripheral.address + ', reconnecting');
    peripheral.disconnect();
    startScanning();
  });

  // we will try to connect during 1 minute
  watchdog.next(60 * 1000);

  calypso.connect(peripheral, function(err) {
    if (err) {
      startScanning();
      return;
    }
    var noDataTimeout;
    peripheral.listenData(function (err, sensorData) {
      if (err) {
        console.warn('Device error, trying to reconnect: ', err);
        calypso.scan(peripheralDiscovered);
        return;
      }
      var source = 'Calypso ' + peripheral.address;
      var values = anemonode.dispatcher.values;
      values.awa.setValue(source, sensorData.awa);
      values.aws.setValue(source, sensorData.aws);
      values.roll.setValue(source, sensorData.roll);
      values.pitch.setValue(source, sensorData.pitch);
      values.magHdg.setValue(source, sensorData.heading);

      // If we stop receiving data for 5 seconds, we start scanning
      // for another device.
      watchdog.next(5 * 1000);
    });
  });
}

calypso.setDiscoveredCallback(peripheralDiscovered);

function startScanning() {
  calypso.scan(function(err) { console.warn(err); });
}

module.exports.startScanning = startScanning;

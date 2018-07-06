var calypso = require('calypso_ultrasonic');
var anemonode = require('../build/Release/anemonode');

var connectedPeripheral = undefined;
var supportEnabled = false;

function Watchdog(cb) {
  var t = this;
  this.cb = function() { t.timeout = undefined; cb(); };
  this.timeout = undefined;
}

Watchdog.prototype.cancel = function() {
  if (this.timeout) {
    clearTimeout(this.timeout);
    this.timeout = undefined;
  }
}

Watchdog.prototype.next = function(t) {
  this.cancel();
  this.timeout = setTimeout(this.cb, t);
};

// In case of timeout, we close the connection and start scanning again.
var watchdog = new Watchdog(function() {
  if (connectedPeripheral) {
    console.warn('No more data from calypso ' + connectedPeripheral.address
                 + ', reconnecting');
    connectedPeripheral.disconnect();
    connectedPeripheral = undefined;
  }
  startScanning();
});

function peripheralDiscovered(err, peripheral) {
  if (err) {
    console.warn(err);
    return;
  }
  console.log('found calypso ultrasonic: ', peripheral.id);

  calypso.stopScanning();

  connectedPeripheral = peripheral;

  // we will try to connect during 1 minute
  watchdog.next(60 * 1000);

  calypso.connect(peripheral, function(err) {
    if (err) {
      console.warn('Error connecting to Calypso Ultrasonic');
      watchdog.cancel();
      connectedPeripheral = undefined;
      startScanning();
      return;
    }

    console.log('Connected to Calypso Ultrasonic ' + peripheral.address);

    var noDataTimeout;
    peripheral.listenData(function (err, sensorData) {
      if (err) {
        watchdog.cancel();
        console.warn('Device error, trying to reconnect: ', err);
        connectedPeripheral = undefined;
        peripheral.disconnect();
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
  if (supportEnabled) {
    calypso.scan(function(err) { console.warn(err); });
  }
}

function start() {
  if (!supportEnabled) {
    console.log('Starting up Calypso Ultrasonic support');
  }
  supportEnabled = true;
  startScanning();
}

function shutdown() {
  if (supportEnabled) {
    supportEnabled = false;

    console.log('Shutting down Calypso Ultrasonic support');
    watchdog.cancel();
    if (connectedPeripheral) {
      connectedPeripheral.disconnect();
      connectedPeripheral = undefined;
    }
    calypso.stopScanning();
  }
}

module.exports.start = start;
module.exports.shutdown = shutdown;

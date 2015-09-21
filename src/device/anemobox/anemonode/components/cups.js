var debug = require('debug')('cups');
var async = require('async');
var noble = require('noble');
var anemonode = require('../build/Release/anemonode');

var sourceName = "CUPS";

noble.on('stateChange', function(state) {
  debug('State: ' + state);
  if (state === 'poweredOn') {
    noble.startScanning();
  } else {
    noble.stopScanning();
  }
});

noble.on('discover', function(peripheral) {
  debug('peripheral with ID: ' + peripheral.id);
  var advertisement = peripheral.advertisement;
  var localName = advertisement.localName;
  var txPowerLevel = advertisement.txPowerLevel;
  var manufacturerData = advertisement.manufacturerData;
  var serviceData = advertisement.serviceData;
  var serviceUuids = advertisement.serviceUuids;

  if (localName) {
    debug('  Local Name        = ' + localName);
  }

  if (txPowerLevel) {
    debug('  TX Power Level    = ' + txPowerLevel);
  }

  if (manufacturerData) {
    debug('  Manufacturer Data = ' + manufacturerData.toString('hex'));
  }

  if (serviceData) {
    debug('  Service Data      = ' + serviceData);
  }

  if (localName) {
    debug('  Service UUIDs     = ' + serviceUuids);
  }

  if (/^CUPS4\.[0-9]+\.[0-9]+$/.test(localName)) {
    noble.stopScanning();
    debug(localName + ' with ID ' + peripheral.id + ' found');
    connectToCUPS(peripheral);
  }
});

function subscribeToCups(peripheral, char) {
  char.notify(true, function(error) {
    if (error) {
      debug('Failed to subscribe to CUPS notifications: ' + error);
      peripheral.disconnect();
    }
  });
  char.on('notify', function(state) {
    debug('CUPS notify subscription: ' + state);
  });

  var lastWindSpeed = -1;
  var lastWindSpeedCount = 0;

  char.on('data', function(data, isNotification) {
    var windSpeed = data.readUInt16LE(0) / 100;
    var windDir = data.readUInt16LE(2);
    var battery = data.readUInt16LE(4);
    debug(windSpeed + ' m/s, ' + windDir + ' battery: ' + battery + ' mV');
    var meterPerSecToKnots = 1.94384449;

    // When the wind sensor stops to rotate, the value should drop to 0 m/s.
    // However, it remains at the same value.
    // If we read 20 times the same value, we consider the wind as invalid.
    if (lastWindSpeed == windSpeed) {
      ++lastWindSpeedCount;
    } else {
      lastWindSpeedCount = 0;
      lastWindSpeed = windSpeed;
    }
    if (lastWindSpeedCount >= 10) {
      windSpeed = 0;
    }
    anemonode.dispatcher.values.awa.setValue(sourceName, windDir);
    anemonode.dispatcher.values.aws.setValue(sourceName, windSpeed * meterPerSecToKnots);
  });
}

function connectToCUPS(peripheral) {
  debug('services and characteristics:');

  peripheral.on('disconnect', function() {
    debug('CUPS Disconnected, scanning again');
    noble.startScanning();
  });

  peripheral.connect(function(error) {
    if (error) {
      debug('Error connection to CUPS: ' + error);
      return;
    }
    debug('Connected to CUPS: searching for wind characteristics');

    var serviceUUIDs = ["0000180d1212efde1523785feabcd123"];
    var characteristicUUIDs = ["00002a391212efde1523785feabcd123"];
    peripheral.discoverSomeServicesAndCharacteristics(serviceUUIDs, characteristicUUIDs, function(error, services, characteristics) {
      debug('Found characteristics:');
      if (characteristics.length == 1) {
        subscribeToCups(peripheral, characteristics[0]);
      } else {
        peripheral.disconnect();
      }
    });
  });
}


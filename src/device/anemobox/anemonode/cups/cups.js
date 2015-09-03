var async = require('async');
var noble = require('noble');

noble.on('stateChange', function(state) {
  console.log('State: ' + state);
  if (state === 'poweredOn') {
    noble.startScanning();
  } else {
    noble.stopScanning();
  }
});

noble.on('discover', function(peripheral) {
  console.log('peripheral with ID: ' + peripheral.id);
    var advertisement = peripheral.advertisement;
    var localName = advertisement.localName;
    var txPowerLevel = advertisement.txPowerLevel;
    var manufacturerData = advertisement.manufacturerData;
    var serviceData = advertisement.serviceData;
    var serviceUuids = advertisement.serviceUuids;

    if (localName) {
      console.log('  Local Name        = ' + localName);
    }

    if (txPowerLevel) {
      console.log('  TX Power Level    = ' + txPowerLevel);
    }

    if (manufacturerData) {
      console.log('  Manufacturer Data = ' + manufacturerData.toString('hex'));
    }

    if (serviceData) {
      console.log('  Service Data      = ' + serviceData);
    }

    if (localName) {
      console.log('  Service UUIDs     = ' + serviceUuids);
    }

    console.log();

  if (localName === "CUPS4.0.0") {
    noble.stopScanning();

    console.log('CUPS with ID ' + peripheral.id + ' found');


    connectToCUPS(peripheral);
  }
});

function subscribeToCups(peripheral, char) {
  char.notify(true, function(error) {
    if (error) {
      console.log('Failed to subscribe to CUPS notifications: ' + error);
      peripheral.disconnect();
    }
  });
  char.on('notify', function(state) {
    console.log('CUPS notify subscription: ' + state);
  });
  char.on('data', function(data, isNotification) {
    var windSpeed = data.readUInt16LE(0) / 100;
    var windDir = data.readUInt16LE(2);
    var battery = data.readUInt16LE(4);
    console.log(windSpeed + ' m/s, ' + windDir + ' battery: ' + battery + ' mV');
  });
}

function connectToCUPS(peripheral) {
  console.log('services and characteristics:');

  peripheral.on('disconnect', function() {
    console.log('CUPS Disconnected, scanning again');
    noble.startScanning();
  });

  peripheral.connect(function(error) {
    if (error) {
      console.log('Error connection to CUPS: ' + error);
      return;
    }
    console.log('Connected to CUPS: searching for wind characteristics');

    if (1) {
    var serviceUUIDs = ["0000180d1212efde1523785feabcd123"];
    var characteristicUUIDs = ["00002a391212efde1523785feabcd123"];
    peripheral.discoverSomeServicesAndCharacteristics(serviceUUIDs, characteristicUUIDs, function(error, services, characteristics) {
      console.log('Found characteristics:');
      console.warn(characteristics);
      if (characteristics.length == 1) {
        subscribeToCups(peripheral, characteristics[0]);
      } else {
        peripheral.disconnect();
      }
    });
    }

    if (0) {
    peripheral.discoverServices([], function(error, services) {
      var serviceIndex = 0;

      async.whilst(
        function () {
          return (serviceIndex < services.length);
        },
        function(callback) {
          var service = services[serviceIndex];
          var serviceInfo = service.uuid;

          if (service.name) {
            serviceInfo += ' (' + service.name + ')';
          }
          console.log(serviceInfo);

          service.discoverCharacteristics([], function(error, characteristics) {
            var characteristicIndex = 0;

            async.whilst(
              function () {
                return (characteristicIndex < characteristics.length);
              },
              function(callback) {
                var characteristic = characteristics[characteristicIndex];
                var characteristicInfo = '  ' + characteristic.uuid;

                if (characteristic.name) {
                  characteristicInfo += ' (' + characteristic.name + ')';
                }

                async.series([
                  function(callback) {
                    characteristic.discoverDescriptors(function(error, descriptors) {
                      async.detect(
                        descriptors,
                        function(descriptor, callback) {
                          return callback(descriptor.uuid === '2901');
                        },
                        function(userDescriptionDescriptor){
                          if (userDescriptionDescriptor) {
                            userDescriptionDescriptor.readValue(function(error, data) {
                              if (data) {
                                characteristicInfo += ' (' + data.toString() + ')';
                              }
                              callback();
                            });
                          } else {
                            callback();
                          }
                        }
                      );
                    });
                  },
                  function(callback) {
                        characteristicInfo += '\n    properties  ' + characteristic.properties.join(', ');

                    if (characteristic.properties.indexOf('read') !== -1) {
                      characteristic.read(function(error, data) {
                        if (data) {
                          var string = data.toString('ascii');

                          characteristicInfo += '\n    value       ' + data.toString('hex') + ' | \'' + string + '\'';
                        }
                        callback();
                      });
                    } else {
                      callback();
                    }
                  },
                  function() {
                    console.log(characteristicInfo);
                    characteristicIndex++;
                    callback();
                  }
                ]);
              },
              function(error) {
                serviceIndex++;
                callback();
              }
            );
          });
        },
        function (err) {
          peripheral.disconnect();
        }
      );
    });
    }
  });
}

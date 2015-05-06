var util = require('util'),
  exec = require('child_process').exec,
  bleno = require('bleno'),
  Descriptor = bleno.Descriptor,
  Characteristic = bleno.Characteristic;

/**
* Reference:
* https://developer.bluetooth.org/gatt/characteristics/Pages/CharacteristicViewer.aspx?u=org.bluetooth.characteristic.manufacturer_name_string.xml
*/
var DeviceManufacturerCharacteristic = function() {
  DeviceManufacturerCharacteristic.super_.call(this, {
      // Device Manufacturer
      uuid: '2A29',
      properties: ['read'],
      value: new Buffer('Anemomind','utf8'),
      descriptors: [
        new Descriptor({
            uuid: '2901',
            value: 'Device ID'
        })
      ]
  });
};

util.inherits(DeviceManufacturerCharacteristic, Characteristic);

module.exports = DeviceManufacturerCharacteristic;

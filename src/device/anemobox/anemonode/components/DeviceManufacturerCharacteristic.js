/**
* Reference: https://developer.bluetooth.org/gatt/services/Pages/ServiceViewer.aspx?u=org.bluetooth.service.device_information.xml
*/

var util = require('util'),
  exec = require('child_process').exec,
  bleno = require('bleno'),
  Descriptor = bleno.Descriptor,
  Characteristic = bleno.Characteristic,
  anemoId = require('./boxId'),
  anemoIdBuffer;

anemoId.getAnemoId(function(id) {
  anemoIdBuffer = new Buffer(id, 'utf8');
});

/**
* Reference:
* https://developer.bluetooth.org/gatt/characteristics/Pages/CharacteristicViewer.aspx?u=org.bluetooth.characteristic.manufacturer_name_string.xml
*/
var DeviceManufacturerCharacteristic = function() {
  DeviceManufacturerCharacteristic.super_.call(this, {
      // Device Manufacturer
      uuid: '2A29',
      properties: ['read'],
      descriptors: [
        new Descriptor({
            uuid: '2901',
            value: 'Device ID'
        })
      ]
  });
};

util.inherits(DeviceManufacturerCharacteristic, Characteristic);

DeviceManufacturerCharacteristic.prototype.onReadRequest = function(offset, callback) {
  callback(this.RESULT_SUCCESS, anemoIdBuffer);
};

module.exports = DeviceManufacturerCharacteristic;

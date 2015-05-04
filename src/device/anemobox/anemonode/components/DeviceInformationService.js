/**
* Reference:
* https://developer.bluetooth.org/gatt/characteristics/Pages/CharacteristicViewer.aspx?u=org.bluetooth.characteristic.manufacturer_name_string.xml
*/
var util = require('util'),
  bleno = require('bleno'),
  BlenoPrimaryService = bleno.PrimaryService,
  DeviceManufacturerCharacteristic = require('./DeviceManufacturerCharacteristic');

function startBTLE() {
	function DeviceInformationService() {
	  DeviceInformationService.super_.call(this, {
	      uuid: '180A',
	      characteristics: [
	          new DeviceManufacturerCharacteristic()
	      ]
	  });
	}

	util.inherits(DeviceInformationService, BlenoPrimaryService);
	module.exports = DeviceInformationService;
}

module.exports.startBTLE = startBTLE;

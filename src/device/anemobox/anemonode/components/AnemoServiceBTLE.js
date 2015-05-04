var util = require('util');
var bleno = require('bleno');
var DeviceManufacturerCharacteristic = require('./DeviceManufacturerCharacteristic');

var BlenoPrimaryService = bleno.PrimaryService;
var BlenoCharacteristic = bleno.Characteristic;


function startBTLE() {

  var characteristicsArray = [];
  require('./DispatcherBLE').pushCharacteristics(characteristicsArray);
  require('./ConfigBLE').pushCharacteristics(characteristicsArray);

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
   
  function AnemoService() {
    AnemoService.super_.call(this, {
      uuid: 'fffffffffffffffffffffffffffffff0',
      characteristics: characteristicsArray
    });
  }
   
  util.inherits(AnemoService, BlenoPrimaryService);
   
  bleno.on('stateChange', function(state) {
    console.log('on -> stateChange: ' + state);
   
    if (state === 'poweredOn') {
      bleno.startAdvertising('anemomind', ['fffffffffffffffffffffffffffffff0']);
    } else {
      bleno.stopAdvertising();
    }
  });
   
  bleno.on('advertisingStart', function(error) {
    console.log('on -> advertisingStart: ' + (error ? 'error ' + error : 'success'));
   
    if (!error) {
      bleno.setServices([
        new AnemoService(),
        new DeviceInformationService()
      ]);
    }
  });
   
  bleno.on('advertisingStop', function() {
    console.log('on -> advertisingStop');
  });
   
  bleno.on('servicesSet', function() {
    console.log('on -> servicesSet');
  });
}

module.exports.startBTLE = startBTLE;

var util = require('util');
var bleno = require('bleno');
var DeviceManufacturerCharacteristic = require('./DeviceManufacturerCharacteristic');
var SerialNumberCharacteristic = require('./SerialNumberCharacteristic');

var BlenoPrimaryService = bleno.PrimaryService;
var BlenoCharacteristic = bleno.Characteristic;


function startBTLE() {

  var characteristicsArray = [];
  require('./DispatcherBLE').pushCharacteristics(characteristicsArray);
  require('./ConfigBLE').pushCharacteristics(characteristicsArray);
  require('./rpcble').pushCharacteristics(characteristicsArray);

  function DeviceInformationService() {
    DeviceInformationService.super_.call(this, {
        uuid: '180A',
        characteristics: [
            new SerialNumberCharacteristic(),
            new DeviceManufacturerCharacteristic()
        ]
    });
  }

  util.inherits(DeviceInformationService, BlenoPrimaryService);
  module.exports = DeviceInformationService;
   
  var anemoServiceUuid = 'AFF1E42DEF91456F86FA8703FFFFFFF0';
  function AnemoService() {
    AnemoService.super_.call(this, {
      uuid: anemoServiceUuid,
      characteristics: characteristicsArray
    });
  }
   
  util.inherits(AnemoService, BlenoPrimaryService);
   
  bleno.on('stateChange', function(state) {
    console.log('on -> stateChange: ' + state);
   
    if (state === 'poweredOn') {
      bleno.startAdvertising('Anemobox', [anemoServiceUuid]);
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

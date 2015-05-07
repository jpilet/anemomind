var util = require('util');
var bleno = require('bleno');

var BlenoPrimaryService = bleno.PrimaryService;
var BlenoCharacteristic = bleno.Characteristic;

function startBTLE() {

  var characteristicsArray = [];
  require('./DispatcherBLE').pushCharacteristics(characteristicsArray);
  require('./ConfigBLE').pushCharacteristics(characteristicsArray);

   
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
      bleno.startAdvertising('Anemobox', ['fffffffffffffffffffffffffffffff0']);
    } else {
      bleno.stopAdvertising();
    }
  });
   
  bleno.on('advertisingStart', function(error) {
    console.log('on -> advertisingStart: ' + (error ? 'error ' + error : 'success'));
   
    if (!error) {
      bleno.setServices([
        new AnemoService()
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

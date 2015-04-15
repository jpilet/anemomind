var util = require('util');
var bleno = require('bleno');
var anemonode = require('./build/Release/anemonode');

var BlenoPrimaryService = bleno.PrimaryService;
var BlenoCharacteristic = bleno.Characteristic;

var characteristics = {};
var characteristicsArray = [];


for (var entry in anemonode.dispatcher) {
  characteristics[entry] = function() {
    this.entry = entry;
    characteristics[entry].super_.call(this, {
      uuid: '0000000000000000000000000000000'+anemonode.dispatcher[entry].dataCode,
      properties: ['notify', 'read']
    });
  }
  util.inherits(characteristics[entry], BlenoCharacteristic);

  characteristics[entry].prototype.onSubscribe = function(maxValueSize, updateValueCallback) {
    console.log(this.entry +' subscribe');
   
    this.counter = 0;
    this.changeInterval = setInterval(function() {
      var data = new Buffer(this.counter + '', 'utf-8');
      data.write(this.counter+'');
      console.log(entry +' update value: ' + this.counter);
      updateValueCallback(data);
      this.counter++;
    }.bind(this), 500);
  };

  characteristics[entry].prototype.onUnsubscribe = function() {
    console.log(this.entry +' unsubscribe');
   
    if (this.changeInterval) {
      clearInterval(this.changeInterval);
      this.changeInterval = null;
    }
  };
 
  characteristics[entry].prototype.onNotify = function() {
    console.log(this.entry +' on notify');
  };

  characteristicsArray.push(new characteristics[entry]());

}

var SendDataCharacteristic = function() {
  SendDataCharacteristic.super_.call(this, {
    uuid: 'ffffffffffffffffffffffffffffffff',
    properties: ['notify', 'write']
  });
};

util.inherits(SendDataCharacteristic, BlenoCharacteristic);

SendDataCharacteristic.prototype.onSubscribe = function(maxValueSize, updateValueCallback) {
  console.log('SendDataCharacteristic subscribe');
 
  this.counter = 0;
  this.changeInterval = setInterval(function() {
    var data = new Buffer(this.counter + '', 'utf-8');
    data.write(this.counter+'');
    // data.writeUInt32LE(this.counter, 0);
 
    console.log('SendDataCharacteristic update value: ' + this.counter);
    updateValueCallback(data);
    this.counter++;
  }.bind(this), 500);
};
 
SendDataCharacteristic.prototype.onUnsubscribe = function() {
  console.log('SendDataCharacteristic unsubscribe');
 
  if (this.changeInterval) {
    clearInterval(this.changeInterval);
    this.changeInterval = null;
  }
};
 
SendDataCharacteristic.prototype.onNotify = function() {
  console.log('SendDataCharacteristic on notify');
};

var finalData = '';

SendDataCharacteristic.prototype.onWriteRequest = function(data, offset, withoutResponse, callback) {
  if (offset) {
    callback(this.RESULT_ATTR_NOT_LONG);
  }
  // else if (data.length !== 2) {
  //   callback(this.RESULT_INVALID_ATTRIBUTE_LENGTH);
  // }
  else {
    var receivedData = data.toString('utf8');
    if (receivedData == '==EOM==') {
      console.log(finalData);
      finalData = '';
    } else {
      finalData += receivedData;
    }
    callback(this.RESULT_SUCCESS);
  }
};

characteristicsArray.push(new SendDataCharacteristic());
 
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
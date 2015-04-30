var util = require('util');
var bleno = require('bleno');
var exec = require('child_process').exec;
var anemonode = require('./build/Release/anemonode');

var BlenoPrimaryService = bleno.PrimaryService;
var BlenoCharacteristic = bleno.Characteristic;

var characteristics = {};
var characteristicsArray = [];
var anemoId;

exec("ifconfig wlan0 | grep -o -E '([[:xdigit:]]{1,2}:){5}[[:xdigit:]]{1,2}'", function (error, stdout, stderr) {
    if (stdout) {
      console.log("wlan0 found, anemoID is: " + stdout);
      anemoId = stdout;
    } else {
      console.log("wlan0 not found, anemoID is: " + '78:4b:87:a1:f2:61');
      anemoId = '78:4b:87:a1:f2:61';
    }
  });

//Define a class to adapt dispacher entries to characteristics.
function DispatcherCharacteristic(entry) {
  this.entry = entry;
  DispatcherCharacteristic.super_.call(this, {
    uuid: pad(anemonode.dispatcher[entry].dataCode.toString(16), 32),
    properties: ['notify', 'read']
  });
}
util.inherits(DispatcherCharacteristic, BlenoCharacteristic);

function encodeValueInBuffer(entry) {
  var dispatchData = anemonode.dispatcher[entry];

  function format16bits(value) {
    // is it LE or BE?
    var buffer = new Buffer(2);
    buffer.writeIntLE(value, 0, 2);
    return buffer;
  }
  function format64bits(value) {
    // is it LE or BE?
    var buffer = new Buffer(8);
    buffer.writeIntLE(value, 0, 8);
    return buffer;
  }
  function formatPos(value) {
    // is it LE or BE?
    var buffer = new Buffer(2* 8);
    buffer.writeDoubleLE(value.lon, 0, 8);
    buffer.writeDoubleLE(value.lat, 8, 8);
    return buffer;
  }
  var formatValue = {
    "degrees": format16bits,
    "knots": format16bits,
    "nautical miles": format16bits,
    "WGS84 latitude and longitude, in degrees": formatPos,
    "seconds since 1.1.1970, UTC": format64bits
  };
  
  var unit = dispatchData.unit;
  if (!unit in formatValue) {
    // OK to crash, because all possible units are known at compile time.
    throw('Error: do not know how to format: ' + unit);
  }

  var value = dispatchData.value();
  return formatValue[unit](value);
}

DispatcherCharacteristic.prototype.onSubscribe = function(maxValueSize, updateValueCallback) {
  console.log(this.entry +' subscribe');

  this.subscribeIndex = anemonode.dispatcher[this.entry].subscribe(
    function(val) {
      updateValueCallback(encodeValueInBuffer(entry));
    }
  );
};

DispatcherCharacteristic.prototype.onUnsubscribe = function() {
  console.log(this.entry +' unsubscribe');
 
  if (this.subscribeIndex) {
    anemonode.dispatcher[this.entry].unsubscribe(this.subscribeIndex);
  }
};

DispatcherCharacteristic.prototype.onNotify = function() {
  console.log(this.entry +' on notify');
};

DispatcherCharacteristic.prototype.onReadRequest = function(offset, callback) {
  if (anemonode.dispatcher[this.entry].length() == 0) {
    callback(this.RESULT_UNLIKELY_ERROR);
  } else {
    callback(this.RESULT_SUCCESS, encodeValueInBuffer(this.entry));
  }
};

// Instanciate one DispatcherCharacteristic for each dispatcher entry
for (var entry in anemonode.dispatcher) {
  characteristicsArray.push(new DispatcherCharacteristic(entry));
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

var GetAnemoIdCharacteristic = function() {
  GetAnemoIdCharacteristic.super_.call(this, {
    uuid: 'ffffffffffffffffffffffffffffff00',
    properties: ['read']
  });
};

util.inherits(GetAnemoIdCharacteristic, BlenoCharacteristic);

GetAnemoIdCharacteristic.prototype.onReadRequest = function(offset, callback) {
  console.log("AnemoID characteristic sent: " + anemoId);
  callback(this.RESULT_SUCCESS, new Buffer(anemoId));
};

characteristicsArray.push(new GetAnemoIdCharacteristic());
 
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

function pad (str, max) {
  return str.length < max ? pad("0" + str, max) : str;
}

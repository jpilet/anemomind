var util = require('util');
var bleno = require('bleno');
var anemoId = require('./boxId');
var config = require('./config');

var BlenoPrimaryService = bleno.PrimaryService;
var BlenoCharacteristic = bleno.Characteristic;

var anemoConfigUuidPrefix = 'AFF1E42DEF91456F86FA87031000000';

var GetAnemoIdCharacteristic = function() {
  GetAnemoIdCharacteristic.super_.call(this, {
    uuid: anemoConfigUuidPrefix + '0',
    properties: ['read']
  });
};

util.inherits(GetAnemoIdCharacteristic, BlenoCharacteristic);

GetAnemoIdCharacteristic.prototype.onReadRequest = function(offset, callback) {
  var me = this;
  anemoId.getAnemoId(function(id) {
    console.log('Get box id: ' + id);
    callback(me.RESULT_SUCCESS, new Buffer(id));
  });
};

var BoatIdCharacteristic = function() {
  BoatIdCharacteristic.super_.call(this, {
    uuid: anemoConfigUuidPrefix + '1',
    properties: ['read', 'write']
  });
};

util.inherits(BoatIdCharacteristic, BlenoCharacteristic);

BoatIdCharacteristic.prototype.onReadRequest = function(offset, callback) {
  console.log('read request boatId with offset: ' + offset);
  var me = this;
  config.get(function(err, config) {
    if (config && config.boatId) {
      console.log('Sending boatId:' + config.boatId);
      callback(me.RESULT_SUCCESS, new Buffer(config.boatId));
    } else {
      callback(me.RESULT_UNLIKELY_ERROR);
    }
  });
};

BoatIdCharacteristic.prototype.onWriteRequest = 
    function(data, offset, withoutResponse, callback) {
      console.log('write request to BoatId');
  console.warn((new Error()).stack);
  var me = this;
  if (offset) {
    callback(me.RESULT_ATTR_NOT_LONG);
  } else if (data.length < 10) {
    callback(me.RESULT_INVALID_ATTRIBUTE_LENGTH);
  } else {
    var boatId = data.toString('ascii');
    console.log('assigning boat id: ' + boatId);
    config.change({boatId: boatId}, function(err, cfg) {
      if (err) {
        callback(me.RESULT_UNLIKELY_ERROR);
      } else {
        callback(me.RESULT_SUCCESS);
      }
    });
  }
};

var BoatNameCharacteristic = function() {
  BoatNameCharacteristic.super_.call(this, {
    uuid: anemoConfigUuidPrefix + '2',
    properties: ['read', 'write']
  });
};

util.inherits(BoatNameCharacteristic, BlenoCharacteristic);

BoatNameCharacteristic.prototype.onReadRequest = function(offset, callback) {
  console.log('boat name read request.');
  var me = this;
  config.get(function(err, config) {
    if (config && config.boatName) {
      console.log('Sending boatName:' + config.boatName);
      callback(me.RESULT_SUCCESS, new Buffer(config.boatName));
    } else {
      callback(me.RESULT_UNLIKELY_ERROR);
    }
  });
};

BoatNameCharacteristic.prototype.onWriteRequest = 
    function(data, offset, withoutResponse, callback) {
  if (offset) {
    callback(this.RESULT_ATTR_NOT_LONG);
  } else if (data.length < 3) {
    callback(this.RESULT_INVALID_ATTRIBUTE_LENGTH);
  } else {
    var boatName = data.toString('utf8');
    console.log('changing boat name: ' + boatName);
    config.change({boatName: boatName}, function(err, cfg) {
      if (err) {
        callback(this.RESULT_UNLIKELY_ERROR);
      } else {
        callback(this.RESULT_SUCCESS);
      }
    });
  }
};

module.exports.pushCharacteristics = function(array) {
  array.push(new GetAnemoIdCharacteristic());
  array.push(new BoatIdCharacteristic());
  array.push(new BoatNameCharacteristic());
};



var util = require('util');
var bleno = require('bleno');
var anemoId = require('./boxId');
var config = require('./config');

var BlenoPrimaryService = bleno.PrimaryService;
var BlenoCharacteristic = bleno.Characteristic;

var GetAnemoIdCharacteristic = function() {
  GetAnemoIdCharacteristic.super_.call(this, {
    uuid: 'ffffffffffffffffffffffffffffff00',
    properties: ['read']
  });
};

util.inherits(GetAnemoIdCharacteristic, BlenoCharacteristic);

GetAnemoIdCharacteristic.prototype.onReadRequest = function(offset, callback) {
  anemoId.getAnemoId(function(id) {
    callback(this.RESULT_SUCCESS, new Buffer(id));
  });
};

var BoatIdCharacteristic = function() {
  BoatIdCharacteristic.super_.call(this, {
    uuid: '0001ffffffffffffffffffffffffff00',
    properties: ['read', 'write']
  });
};

util.inherits(BoatIdCharacteristic, BlenoCharacteristic);

BoatIdCharacteristic.prototype.onReadRequest = function(offset, callback) {
  config.get(function(err, config) {
    if (config && config.boatId) {
      callback(this.RESULT_SUCCESS, new Buffer(config.boatId));
    } else {
      callback(this.RESULT_UNLIKELY_ERROR);
    }
  });
};

BoatIdCharacteristic.prototype.onWriteRequest = 
    function(data, offset, withoutResponse, callback) {
  if (offset) {
    callback(this.RESULT_ATTR_NOT_LONG);
  } else if (data.length < 10) {
    callback(this.RESULT_INVALID_ATTRIBUTE_LENGTH);
  } else {
    var boatId = data.toString('ascii');
    console.log('assigning boat id: ' + boatId);
    config.get(function(err, cfg) {
      if (cfg) {
        if (!cfg.boatId || cfg.boatId == boatId) {
          cfg.boatId = boatId;
          config.write(cfg, function(err) {
            if (err) {
              callback(this.RESULT_UNLIKELY_ERROR);
            } else {
              callback(this.RESULT_SUCCESS);
            }
          });
        } else {
          callback(this.RESULT_UNLIKELY_ERROR);
        }
      } else {
        callback(this.RESULT_UNLIKELY_ERROR);
      }
    });
  }
};

var BoatNameCharacteristic = function() {
  BoatNameCharacteristic.super_.call(this, {
    uuid: '0001ffffffffffffffffffffffffff01',
    properties: ['read', 'write']
  });
};

util.inherits(BoatNameCharacteristic, BlenoCharacteristic);

BoatNameCharacteristic.prototype.onReadRequest = function(offset, callback) {
  config.get(function(err, config) {
    if (config && config.boatName) {
      callback(this.RESULT_SUCCESS, new Buffer(config.boatName));
    } else {
      callback(this.RESULT_UNLIKELY_ERROR);
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



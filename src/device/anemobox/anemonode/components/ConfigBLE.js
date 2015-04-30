var util = require('util');
var bleno = require('bleno');
var anemoId = require('./boxId');

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

module.exports.pushCharacteristics = function(array) {
  array.push(new GetAnemoIdCharacteristic());
};



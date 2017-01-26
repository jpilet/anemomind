var express = require('express');
var app = express();
var httpapi = require('../httpapi.js');

app.get('/', function(req, res) {
  res.send('Pine needle tea');
});

function MockEndpoint() {
  this.callCounter = 0;
}

MockEndpoint.prototype.getPacket = function(src, dst, seqNumber, cb) {
  if (src == 'a' && dst == 'b' && seqNumber == 'deadbeef') {
    cb(null, {label: 119, data: new Buffer([13, 21, 34, 55])});
  } else {
    cb('No such packet');
  }
}

var mock = new MockEndpoint();

function mockAccess(f) {
  f(mock, function(err) {});
}

app.use('/mockendpoint', httpapi.make(mockAccess));

module.exports.app = app;

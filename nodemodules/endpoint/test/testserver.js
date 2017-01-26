var express = require('express');
var app = express();
var httpapi = require('../httpapi.js');

app.get('/', function(req, res) {
  res.send('Pine needle tea');
});

function MockEndpoint() {}

MockEndpoint.prototype.getPacket = function(src, dst, seqNumber, cb) {
  if (src == 'a' && dst == 'b' && seqNumber == 'deadbeef') {
    cb(null, {label: 119, data: new Buffer([13, 21, 34, 55])});
  } else {
    cb(new Error('No such packet'));
  }
}

var mock = new MockEndpoint();

function mockAccess(f, cb) {
  f(mock, cb);
}

app.use('/mockendpoint', httpapi.make(mockAccess));

module.exports.app = app;

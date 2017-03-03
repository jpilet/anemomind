var express = require('express');
var app = express();
var httpapi = require('../httpapi.js');
var bodyParser = require('body-parser');

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

MockEndpoint.prototype.putPacket = function(p, cb) {
  if (p.src == 'x' && p.dst == 'y' && p.seqNumber == 'deadbeef'
      && p.label == 119 && p.data.equals(new Buffer([9, 0]))) {
    cb();
  } else {
    cb('Failed to put packet');
  }
}

var mock = new MockEndpoint();

function mockAccess(name, f) {
  if (name == 'mock') {
    f(null, mock, function(err) {/*nothing to clean up*/});
  } else {
    f(new Error('No such endpoint: ' + name));
  }
}

app.use(bodyParser.json());
app.use(bodyParser.raw({type: 'application/octet-stream', limit: '10mb'}));
app.use('/mockendpoint', httpapi.make(express.Router(), mockAccess));

module.exports.app = app;

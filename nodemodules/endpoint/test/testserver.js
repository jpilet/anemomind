var express = require('express');
var app = express();
var httpapi = require('../httpapi.js');

app.get('/', function(req, res) {
  res.send('Pine needle tea');
});

app.use('/mockendpoint', httpapi.make());

module.exports.app = app;

var express = require('express');
var app = express();
var httpapi = require('../httpapi.js');

app.get('/', function(req, res) {
  res.send('Pine needle tea');
});

app.use('/endpoint', httpapi.make());

module.exports = app;

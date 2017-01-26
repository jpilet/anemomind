var express = require('express');
var app = express();

app.get('/', function(req, res) {
  res.send('Pine needle tea');
});

module.exports.app = app;

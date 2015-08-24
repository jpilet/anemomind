var express = require('express');

var app = express();
var server = require('http').createServer(app);
require('./routes')(app);

var server = app.listen(3000, function () {
  var host = server.address().address;
  var port = server.address().port;

  console.log('Http server listening on port %s', port);
});
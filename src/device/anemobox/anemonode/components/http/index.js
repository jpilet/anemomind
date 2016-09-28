var express = require('express');
var bodyParser = require('body-parser');

var app = express();
app.use(bodyParser.json({limit: '10mb'}));

var server = require('http').createServer(app);
require('./routes')(app);

var server = app.listen(80, function () {
  var host = server.address().address;
  var port = server.address().port;

  console.log('Http server listening on port %s', port);
});

module.exports = app;

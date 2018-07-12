/**
 * Main application file
 */

'use strict';

var fs = require('fs');

// Set default node environment to development
process.env.NODE_ENV = process.env.NODE_ENV || 'development';

var express = require('express');
var mongoose = require('mongoose');
var config = require('./config/environment');
var https = require('https');
var morgan = require('morgan');

// Connect to database
mongoose.connect(config.mongo.uri, config.mongo.options);

// Setup server
var app = express();
app.use(morgan());

var server = require('http').createServer(app);
var socketio = require('socket.io')(server, {
  serveClient: true,
  path: '/socket.io-client'
});
var handleSocketIo = require('./config/socketio');
handleSocketIo(socketio);
require('./config/express')(app);
require('./routes')(app);

// Start server
server.listen(config.port, config.ip, function () {
  console.log('Express server listening on %d, in %s mode', config.port, app.get('env'));
});

// Start SSL server
if (config.ssl) {
  var sslserver = https.createServer(
    {
      key: fs.readFileSync(config.ssl.key),
      cert: fs.readFileSync(config.ssl.cert)
    }, app);

  var sslsocketio = require('socket.io')(sslserver, {
    serveClient: (config.env === 'production') ? false : true,
    path: '/socket.io-client'
  });
  handleSocketIo(sslsocketio);

  sslserver.listen(config.ssl.port, config.ssl.ip, function() {
    console.log('HTTPS server listening on %d, in %s mode', config.ssl.port, app.get('env'));
  });
}

// Expose app
exports = module.exports = app;

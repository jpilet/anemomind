/**
 * Main application routes
 */

'use strict';

var express = require('express');

module.exports = function(app) {

  app.use('/api/id', require('./api/id'));
  app.use('/api/live', require('./api/live'));
  app.use('/api/rpc', require('./api/rpc'));
  app.use('/api/config', require('./api/config'));
  app.use('/api/endpoint', require('./api/endpoint'));
  app.use('/', express.static(__dirname + '/static'));

};

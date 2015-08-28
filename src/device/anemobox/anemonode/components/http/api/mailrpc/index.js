'use strict'

var express = require('express');
var router = express.Router();
var rpc = require('endpoint/endpoint.httpserver.js');
var ep = require('../../../LocalEndpoint.js');

function withEndpoint(endpointName, req, cbOperation, done) {
  ep.withNamedLocalEndpointValidated(endpointName, cbOperation, done);
}

rpc.bindMethodHandlers(withEndpoint, router);
module.exports = router;

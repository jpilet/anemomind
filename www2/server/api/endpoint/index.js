// See also: 
// src/device/anemobox/anemonode/components/http/api/endpoint/index.js

'use strict'

var express = require('express');
var httpapi = require('endpoint/httpapi.js');
var auth = require('../../auth/auth.service');
var ep = require('../mailrpc/endpoint.js');

function accessEndpoint(args, f) {
  ep.withEndpoint(args.name, args.req, function(ep, cb) {
    f(null, ep, cb);
  }, function(err) {
    if (err) {
      console.log("Error when accessing endpoint: ");
      console.log(err);
    }
  });
}

var router = express.Router();
router.use(auth.isAuthenticated());
module.exports = httpapi.makeAdvanced(router, accessEndpoint);


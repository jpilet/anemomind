// See also: 
// src/device/anemobox/anemonode/components/http/api/endpoint/index.js

'use strict'

var express = require('express');
var httpapi = require('endpoint/httpapi.js');
var auth = require('../../auth/auth.service');
var ep = require('../mailrpc/endpoint.js');

function accessEndpoint(args, f) {
  var wasCalled = false;
  ep.withEndpoint(args.name, args.req, function(ep, cb) {
    wasCalled = true;
    f(null, ep, cb);
  }, function(err) {
    if (!wasCalled) {
      f(err || new Error("f wasn't called in accessEndpoint"), 
        null, function() {
        console.log("Error when accessing endpoint: ");
      });
    }
  });
}

var router = express.Router();
router.use(auth.isAuthenticated());
module.exports = httpapi.make(router, accessEndpoint);


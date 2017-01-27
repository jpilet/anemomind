'use strict'

var express = require('express');
var httpapi = require('endpoint/httpapi.js');
var lep = require('../../../LocalEndpoint.js');

function accessLocalEndpoint(name, f) {
  lep.withNamedLocalEndpoint(name, function(ep, cb) {
    f(null, ep, cb);
  }, function(err) {
    /* nothing to do after the endpoint was accessed. */
  });
}

module.exports = httpapi.make(accessLocalEndpoint);

'use strict';

var express = require('express');
var router = express.Router();
var rpc = require('endpoint/endpoint.httpserver.js');
var auth = require('../../auth/auth.service');
var ep = require('./endpoint.js');


/*

  Available functions to call, bound to this router.
  This list can be generated using rpc.makeOverview():

  ========= Essential calls ========
  Function name: putPacket
  HTTP-call: POST /putPacket/:endpointName

  Function name: getPacket
  HTTP-call: GET /getPacket/:endpointName/:src/:dst/:seqNumber

  Function name: getSrcDstPairs
  HTTP-call: GET /getSrcDstPairs/:endpointName

  Function name: setLowerBound
  HTTP-call: GET /setLowerBound/:endpointName/:src/:dst/:lowerBound

  Function name: getLowerBounds
  HTTP-call: POST /getLowerBounds/:endpointName

  Function name: getUpperBounds
  HTTP-call: POST /getUpperBounds/:endpointName

  ========= To facilitate unit testing ========
  Function name: getTotalPacketCount
  HTTP-call: GET /getTotalPacketCount/:endpointName

  Function name: sendPacket
  HTTP-call: POST /sendPacket/:endpointName

  Function name: reset
  HTTP-call: GET /reset/:endpointName
  
  All these functions will return JSON data in the body. On success, the
  HTTP status code is 200 and the body data is the result.
  On failure, the status code is 500 and the body data
  is the error.
  
*/

rpc.bindMethodHandlers(ep.withEndpoint, router, auth.isAuthenticated());
module.exports = router;

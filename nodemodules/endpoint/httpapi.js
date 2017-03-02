var util = require('util');
var epstate = require('./epstate.js');

var internalServerError = 500;
var badRequest = 400;

function wrapErrorLogger(errorLogger) {
  return errorLogger == undefined? function(s) {
    console.log(' --> Endpoint HTTP API error: ' + s);
  } : errorLogger;
}

function failure(s) {
  return {failure: s};
}

function success(s) {
  return {success: s};
}

function encodePacket(packet) {
  if (typeof packet.label != 'number') {
    return failure('Packet label is not a number');
  } else if (!(packet.data instanceof Buffer)) {
    return failure('Packet data is not a buffer');
  } else {
    var dst = new Buffer(1 + packet.data.length);
    dst.writeUInt8(packet.label, 0);
    if (packet.label != dst.readUInt8(0)) {
      return failure('Packet label could not be represented as a single byte');
    }
    packet.data.copy(dst, 1, 0);
    return success(dst);
  }
}

function decodePacket(data) {
  if (!(data instanceof Buffer)) {
    return failure("Not a buffer");
  } else if (data.length < 1) {
    return failure("Size of the buffer should be at least 1 (to contain the label)");
  } else {
    return success({
      label: data[0],
      data: data.slice(1)
    });
  }
}

/* 

Make a router for accessing an endpoint.

  * accessEndpoint:
     arg 1: name of endpoint to access
     arg 2: callback, with these arguments:
         arg 1: err (possibly null)
         arg 2: endpoint (possibly null)
         arg 3: A callback that we need to call once we are done using the endpoint

*/

function isString(x) {
  return typeof x == "string";
}

function isRangeQuery(x) {
  if (x instanceof Object) {
    return isString(x.src) && 
      isString(x.dst) && 
      isString(x.lower) && 
      isString(x.upper);
  }
  console.log("Not an object: %j", x);
  return false;
}

function areRangeQueries(x) {
  if (x instanceof Array) {
    for (var i = 0; i < x.length; i++) {
      if (!isRangeQuery(x[i])) {
        console.log("Not a range query: %j", x[i]);
        return false;
      }
    }
    return true;
  }
  console.log("Not an array: %j", x);
  return false;
}

function make(router, accessEndpoint, errorLogger0) {
  var logError = wrapErrorLogger(errorLogger0);

  function withEndpoint(req, res, onAccessErrorData, accessor) {
    accessEndpoint(req.params.name, function(err, endpoint, cb) {
      if (err) {
        logError("Failed to access endpoint: " + err);
        res.status(internalServerError).send(onAccessErrorData);
        cb();
      } else {
        accessor(endpoint, cb);
      }
    });
  }

  var emptyBuffer = new Buffer(0);
  
  /*
    On success:
      Responds with a buffer with at least one byte, where the first byte is the label, 
      and the remaining bytes is the data of the packet.

    On failure:
      Responds with an empty buffer.
  */
  router.get('/getPacket/:name/:src/:dst/:seqNumber', function(req, res) {
    var p = req.params;
    withEndpoint(req, res, emptyBuffer, function(endpoint, cb) {
      endpoint.getPacket(p.src, p.dst, p.seqNumber, function(err, packet) {
        if (err) {
          logError(util.format('getPacket failed with %j', err));
          res.status(internalServerError).send(new Buffer(0));
          cb();
        } else {
          var encoded = encodePacket(packet);
          if (encoded.failure) {
            logError(util.format('Encoding packet failed: %s', encoded.failure));
            res.status(internalServerError).send(new Buffer(0));
            cb();
          } else {
            res.send(encoded.success);
            cb();
          }
        }
      });
    });
  });

  router.put('/putPacket/:name/:src/:dst/:seqNumber', function(req, res) {
    var packetData = decodePacket(req.body);
    if (packetData.success) {
      withEndpoint(req, res, null, function(endpoint, cb) {
        var packet = packetData.success;
        packet.src = req.params.src;
        packet.dst = req.params.dst;
        packet.seqNumber = req.params.seqNumber;
        endpoint.putPacket(packet, function(err) {
          if (err) {
            res.status(internalServerError).send();
            logError("Failed to put packet in endpoint");
            cb();
          } else {
            res.send();
            cb();
          }
        });
      });
    } else {
      console.log("Failed to decode packet in putPacket: %j", 
                  packetData);
      res.status(badRequest).send();
    }
  });

  // Hesitant if I should call it just 'summary', but 'getSummary' is more clear and
  // consistent with 'getPacket'...
  router.get('/getSummary/:name', function(req, res) { 
    withEndpoint(req, res, {}, function(endpoint, cb) {
      epstate.getEndpointSummary(endpoint, function(err, summary) {
        if (err) {
          logError("Failed to get endpoint summary: " + err);
          res.status(internalServerError).json({});
          cb();
        } else {
          res.json(summary);
          cb();
        }
      });
    });
  });

  router.post('/getRangeSizes/:name', function(req, res) {
    withEndpoint(req, res, [], function(endpoint, cb) {
      var queries = req.body.queries;
      if (!areRangeQueries(queries)) {
        res.status(badRequest).json([]);
        cb();
      } else {
        endpoint.getRangeSizes(queries, function(err, data) {
          if (err) {
            logError("Failed to get range sizes");
            res.status(internalServerError).json([]);
            cb();
          } else {
            res.json(data);
            cb();
          }
        });
      }
    });
  });

  return router;
}

module.exports.make = make;
module.exports.encodePacket = encodePacket;
module.exports.decodePacket = decodePacket;

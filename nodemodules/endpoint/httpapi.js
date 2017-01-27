var express = require('express');
var util = require('util');

var internalServerError = 500;

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

/* 

Make a router for accessing an endpoint.

  * accessEndpoint:
     arg 1: name of endpoint to access
     arg 2: callback, with these arguments:
         arg 1: err (possibly null)
         arg 2: endpoint (possibly null)
         arg 3: A callback that we need to call once we are done using the endpoint

*/
function make(accessEndpoint, errorLogger0) {
  var logError = wrapErrorLogger(errorLogger0);
  var router = express.Router();

  /*
    On success:
      Responds with a buffer with at least one byte, where the first byte is the label, 
      and the remaining bytes is the data of the packet.

    On failure:
      Responds with an empty buffer.
  */
  router.get('/getPacket/:name/:src/:dst/:seqNumber', function(req, res) {
    var p = req.params;
    accessEndpoint(p.name, function(err, endpoint, cb) {
      if (err) {
        logError("Failed to access endpoint: " + err);
        res.status(internalServerError).send(new Buffer(0));
        cb();
      } else {
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
      }
    });
  });
  return router;
}

module.exports.make = make;
module.exports.encodePacket = encodePacket;

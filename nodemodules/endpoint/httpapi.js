var express = require('express');
var util = require('util');

var internalServerError = 500;

function wrapErrorLogger(errorLogger) {
  return errorLogger == undefined? function(s) {
    console.log(' --> Endpoint HTTP API error: ' + s);
  } : errorLogger;
}

/* 
Make a router for accessing an endpoint.
In informal syntax, 

accessEndpoint: ((endpoint, (err)->()) -> ()) -> ()

accessEndpoint is a function that takes as input a function which will be called
with the endpoint, and a callback that we should call once we have done what
we want with the endpoint.
*/

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

function make(accessEndpoint, errorLogger0) {
  var logError = wrapErrorLogger(errorLogger0);
  var router = express.Router();

  router.get('/info', function(req, res) {
    res.send('endpoint');
  });

  /*
    On success:
      Responds with a buffer with at least one byte, where the first byte is the label, 
      and the remaining bytes is the data of the packet.

    On failure:
      Responds with an empty buffer.
  */
  router.get('/getpacket/:src/:dst:/:seqNumber', function(req, res) {

    accessEndpoint(function(endpoint, cb) {
      endpoint.getPacket(src, dst, seqNumber, function(err, packet) {
        if (err) {
          logError(util.format('getPacket failed with %j', err));
          res.status(internalServerError).send(new Buffer(0));
        } else {
          var encoded = encodePacketResponse(packet);
          if (encoded.failure) {
            logError(util.format('Encoding packet failed: %s', encoded.failure));
            res.status(internalServerError).send(new Buffer(0));
          } else {
            res.send(encoded.success);
          }
        }
      });

      // This will close it.
      cb();
    });
  });
  return router;
}

module.exports.make = make;
module.exports.encodePacket = encodePacket;

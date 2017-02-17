var ep = require('./endpoint.sqlite.js');
var assert = require('assert');


function getAllBounds(endpoint, pairs, index, cb) {
  if (index == pairs.length) {
    cb(null, pairs);
  } else {
    var pair = pairs[index];
    endpoint.getPacketBounds(pair.src, pair.dst, function(err, bd) {
      if (err) {
        cb(err);
      } else {
        if (bd) {
          pairs[index].lower = bd.lower;
          pairs[index].upper = bd.upper;
        }
        getAllBounds(endpoint, pairs, index+1, cb);
      }
    });
  }
}

function summarizePackets(endpoint, cb) {
  endpoint.getSrcDstPairs(function(err, pairs) {
    if (err) {
      cb(err);
    } else {
      getAllBounds(endpoint, pairs, 0, cb);
    }
  });
}

function getEndpointSummary(endpoint, cb) {
  ep.getAllFromTable(endpoint.db, 'lowerBounds', function(err, lowerBounds) {
    if (err) {
      cb(err);
    } else {
      summarizePackets(endpoint, function(err, packetSummary) {
        if (err) {
          cb(err);
        } else {
          cb(null, {
            lowerBounds: lowerBounds,
            packets: packetSummary
          });
        }
      });
    }
  });
}

function dispStateOp(endpoint, op, cb) {
  console.log('================ Operation on endpoint');
  getEndpointSummary(endpoint, function(err, data) {
    if (err) {return cb(err);}
    console.log('== Endpoint state before operation:\n  ' + 
                JSON.stringify(data, null, 2));
    op(function(err, result) {
      if (err) {return cb(err);}
      getEndpointSummary(endpoint, function(err, data) {
        if (err) {return cb(err);}
        console.log('== Endpoint state after operation:\n  ' + 
                    JSON.stringify(data, null, 2));
        cb(null, result);
      });
    });
  });
}


module.exports.getEndpointSummary = getEndpointSummary;
module.exports.dispStateOp = dispStateOp;

var assert = require('assert');
var endpoint = require('./endpoint.sqlite.js');
var sync2 = require('./sync2.js');
var endpointh = require('./endpoint.httpclient.js');
var common = require('./common.js');
var schema = require('./endpoint-schema.js');


var testuser = {
    'email': 'test@anemomind.com',
    'password': 'anemoTest'
};

var address = 'http://localhost:9000';


function makeA(cb) {
  endpoint.tryMakeAndResetEndpoint('/tmp/epa.db', 'a', cb);
}

function makeB(cb) {
  endpoint.tryMakeEndpoint(address, testuser, 'b', function(err, ep) {
    if (err) {
      cb(err);
    } else {
      ep.reset(function(err) {
        if (err) {
          cb(err);
        } else {
          cb(null, ep);
        }
      });
    }
  });
}

function makeAB(cb) {
  makeA(function(err, A) {
    if (err) {
      cb(err);
    } else {
      makeB(function(err, B) {
        if (err) {
          cb(err);
        } else {
          schema.makeVerbose(A);
          schema.makeVerbose(B);
          cb(null, A, B);
        }
      });
    }
  });
}


makeAB(function(err, a, b) {
  var ap = [];
  a.addPacketHandler(function(endpoint, packet) {ap.push(packet);});
  assert(!err);
  a.sendPacket('b', 119, new Buffer([0, 3, 4]), function(err) {
    assert(!err);
    a.sendPacket('b', 120, new Buffer([3, 3]), function(err) {
      assert(!err);
      b.sendPacket('a', 121, new Buffer([112]), function(err) {
        assert(!err);
        sync2.synchronize(a, b, function(err) {
          assert(!err);
          assert.equal(ap.length, 1);
          assert(ap[0].data.equals(new Buffer([112])));
          sync2.synchronize(a, b, function(err) {
            assert(!err);
            assert.equal(ap.length, 1);
            a.getTotalPacketCount(function(err, count) {
              assert(!err);
              assert.equal(count, 0);
              b.getTotalPacketCount(function(err, count) {
                assert(!err);
                assert.equal(count, 0);
                console.log('SUCCESSFUL SYNCHRONIZATION WITH SERVER!!!');
              });
            });
          });
        });
      });
    });
  });
});

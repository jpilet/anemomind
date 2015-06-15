var assert = require('assert');
var mail2 = require('./mail2.sqlite.js');
var sync2 = require('./sync2.js');
var mail2h = require('./mail2.http.js');
var common = require('./common.js');
var schema = require('./endpoint-schema.js');

var makeBuf = common.makeBuf;

var testuser = {
    'email': 'test@anemomind.com',
    'password': 'anemoTest'
};

var address = 'http://localhost:9000';


function makeA(cb) {
  mail2.tryMakeAndResetEndPoint('/tmp/epa.db', 'a', cb);
}

function makeB(cb) {
  mail2h.tryMakeEndPoint(address, testuser, 'b', function(err, ep) {
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
  a.addPacketHandler(function(ep, p) {ap.push(p);});
  assert(!err);
  a.sendPacket('b', 119, makeBuf([0, 3, 4]), function(err) {
    assert(!err);
    a.sendPacket('b', 120, makeBuf([3, 3]), function(err) {
      assert(!err);
      b.sendPacket('a', 121, makeBuf([112]), function(err) {
        assert(!err);
        sync2.synchronize(a, b, function(err) {
          assert(!err);
          assert.equal(ap.length, 1);
          assert(ap[0].data.equals(makeBuf([112])));
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

var sync2 = require('../sync2.js');
var mail2 = require('../mail2.sqlite.js');
var assert = require('assert');

function makeA(cb) {
  mail2.tryMakeAndResetEndPoint('/tmp/epa.db', 'a', cb);
}

function makeB(cb) {
  mail2.tryMakeAndResetEndPoint('/tmp/epb.db', 'b', cb);
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
          cb(null, A, B);
        }
      });
    }
  });
}

function makeBuf(x) {
  var dst = new Buffer(x.length);
  for (var i = 0; i < x.length; i++) {
    dst[i] = i;
  }
  return dst;
}


describe('sync2', function() {
  var aPackets = [];
  var bPackets = [];
  
  it('Should synchronize one packet', function(done) {
    makeAB(function(err, a, b) {
      assert(!err);
      a.addPacketHandler(function(p) {aPackets.push(p);});
      b.addPacketHandler(function(p) {bPackets.push(p);});
      a.sendPacket('b', 119, makeBuf([0, 1, 2]), function(err) {
        assert(!err);
        sync2.synchronize(a, b, function(err) {
          assert(!err);
          assert.equal(aPackets.length, 0);
          assert.equal(bPackets.length, 1);
          done();
        });
      });
    });
  });
});

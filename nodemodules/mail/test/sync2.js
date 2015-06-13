var sync2 = require('../sync2.js');
var mail2 = require('../mail2.sqlite.js');
var assert = require('assert');
var eq = require('deep-equal-ident');
var common = require('../common.js');

var makeBuf = common.makeBuf;

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

describe('sync2', function() {
  it('Should synchronize one packet', function(done) {
    var aPackets = [];
    var bPackets = [];
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
  
  it('synco', function(done) {
    var aPackets = [];
    var bPackets = [];
    makeAB(function(err, a, b) {
      assert(!err);
      a.addPacketHandler(function(p) {aPackets.push(p);});
      b.addPacketHandler(function(p) {bPackets.push(p);});
      b.sendPacket('a', 119, makeBuf([0, 1, 2]), function(err) {
        assert(!err);
        sync2.synchronize(a, b, function(err) {
          assert(!err);
          assert.equal(aPackets.length, 1);
          assert.equal(bPackets.length, 0);
          sync2.synchronize(a, b, function(err) {
            assert(!err);
            assert.equal(aPackets.length, 1);
            assert.equal(bPackets.length, 0);
            a.getTotalPacketCount(function(err, count) {
              assert(!err);
              assert.equal(count, 0);
              b.getTotalPacketCount(function(err, count) {
                assert(!err);
                assert.equal(count, 0);
                done();
              });
            });
          });
        });
      });
    });
  });

  it('syncmany', function(done) {
    makeAB(function(err, a, b) {
      var ap = [];
      var bp = [];
      a.addPacketHandler(function(p) {ap.push(p);});
      b.addPacketHandler(function(p) {bp.push(p);});
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
              assert.equal(bp.length, 2);
              assert(ap[0].data.equals(makeBuf([112])));
              sync2.synchronize(a, b, function(err) {
                assert(!err);
                assert.equal(ap.length, 1);
                assert.equal(bp.length, 2);
                a.getTotalPacketCount(function(err, count) {
                  assert(!err);
                  assert.equal(count, 0);
                  b.getTotalPacketCount(function(err, count) {
                    assert(!err);
                    assert.equal(count, 0);
                    done();
                  });
                });
              });
            });
          });
        });
      });
    });
  });
});

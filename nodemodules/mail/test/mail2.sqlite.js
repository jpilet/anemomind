var mb = require('../mail2.sqlite.js');
var assert = require('assert');
var bigint = require('../bigint.js');

function makeTestEP(cb) {
  mb.tryMakeAndResetEndPoint('/tmp/ep.db', 'ep', cb);
}

describe('EndPoint', function() {
  it('Should instantiate a new end point without problems', function(done) {
    mb.tryMakeEndPoint("/tmp/newendpt.js", "newendpt", function(err, ep) {
      assert(!err);
      assert(ep instanceof mb.EndPoint);
      assert.equal(ep.name, "newendpt");
      done();
    });
  });
  
  it('Should instantiate a new end point without problems, and reset it', function(done) {
    mb.tryMakeAndResetEndPoint("/tmp/newendpt.js", "newendpt", function(err, ep) {
      assert(!err);
      assert(ep instanceof mb.EndPoint);
      assert.equal(ep.name, "newendpt");
      done();
    });
  });

  it('Should get the lower bound of an empty end point', function(done) {
    makeTestEP(function(err, ep) {
      ep.getLowerBound('a', 'b', function(err, lb) {
        assert(!err);
        assert.equal(lb, bigint.zero());
        done();
      });
    });
  });
  
  it('Try to get a packet from an empty db', function(done) {
    makeTestEP(function(err, ep) {
      ep.getPacket('a', 'b', '000', function(err, packet) {
        assert(!err);
        assert(!packet);
        done();
      });
    });
  });
  
  it('Get the upper bound from an empty db', function(done) {
    makeTestEP(function(err, ep) {
      ep.getUpperBound('a', 'b', function(err, ub) {
        assert(!err);
        assert.equal(ub, bigint.zero());
        done();
      });
    });
  });
  
  it('Send a packet, get lower and upper bounds, set lower bound', function(done) {
    makeTestEP(function(err, ep) {
      ep.sendPacketAndReturn('b', 119, new Buffer(3), function(err, packet) {
        assert(!err);
        assert.equal(packet.src, ep.name);
        assert.equal(packet.label, 119);
        assert(bigint.zero() < packet.seqNumber);
        assert(packet.data instanceof Buffer);
        ep.getUpperBound('ep', 'b', function(err, ub) {
          assert(!err);
          assert.equal(ub, bigint.inc(packet.seqNumber));
          ep.getLowerBound('ep', 'b', function(err, lb) {
            assert(!err);
            assert.equal(lb, packet.seqNumber);
            ep.setLowerBound('ep', 'b', packet.seqNumber, function(err) {
              assert(!err);
              ep.getLowerBound('ep', 'b', function(err, lb) {
                assert(!err);
                assert.equal(lb, packet.seqNumber);
                ep.getTotalPacketCount(function(err, count) {
                  assert(!err);
                  assert(count == 1);
                  ep.setLowerBound('ep', 'b', bigint.inc(packet.seqNumber), function(err) {
                    assert(!err);
                    ep.getTotalPacketCount(function(err, count) {
                      assert(!err);
                      assert(count == 0);
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
});

var mb = require('../mail2.sqlite.js');
var assert = require('assert');
var bigint = require('../bigint.js');
var eq = require('deep-equal-ident');

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

  it('Send a two packets, set the lower bound for one of them', function(done) {
    makeTestEP(function(err, ep) {
      ep.sendPacketAndReturn('a', 119, new Buffer(0), function(err, pa) {
        assert(!err);
        ep.sendPacketAndReturn('b', 119, new Buffer(0), function(err, pb) {
          assert(!err);
          ep.getTotalPacketCount(function(err, count) {
            assert(!err);
            assert.equal(count, 2);
            ep.setLowerBound('ep', 'b', bigint.inc(pb.seqNumber), function() {
              ep.getTotalPacketCount(function(err, count) {
                assert(!err);
                assert.equal(count, 1);
                ep.getPacket('ep', 'a', pa.seqNumber, function(err, pa2) {
                  assert(!err);
                  assert(eq(pa, pa2));
                  ep.getPacket('ep', 'b', pb.seqNumber, function(err, pb2) {
                    assert(!err);
                    assert(!pb2);
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

  it('Compute the union', function() {
    var A = [{src:'a', dst:'b'}, {src:'a', dst:'c'}, {src:'b', dst:'a'}];
    var B = [{src:'a', dst:'c'}, {src:'c', dst:'a'}];
    var C = mb.srcDstPairUnion(A, B);
    assert(eq(C, [{src:'a', dst:'b'}, {src:'a', dst:'c'},
                  {src:'b', dst:'a'}, {src:'c', dst:'a'}]));
  });
  
  it('Compute the intersection', function() {
    var A = [{src:'a', dst:'b'}, {src:'a', dst:'c'}, {src:'b', dst:'a'}];
    var B = [{src:'a', dst:'c'}, {src:'c', dst:'a'}];
    var C = mb.srcDstPairIntersection(A, B);
    assert(eq(C, [{src:'a', dst:'c'}]));
  });
  
  it('Send a two packets, get the sorted src,dst pairs', function(done) {
    makeTestEP(function(err, ep) {
      ep.sendPacketAndReturn('b', 119, new Buffer(0), function(err, pa) {
        assert(!err);
        ep.sendPacketAndReturn('a', 119, new Buffer(0), function(err, pb) {
          assert(!err);
          done();
        });
      });
    });
  });
});

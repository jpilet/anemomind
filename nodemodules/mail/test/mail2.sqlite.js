var mail2 = require('../mail2.sqlite.js');
var assert = require('assert');
var bigint = require('../bigint.js');
var eq = require('deep-equal-ident');
var schema = require('../endpoint-schema.js');

function makeTestEP(cb) {
  mail2.tryMakeAndResetEndPoint('/tmp/ep.db', 'ep', cb);
}

describe('EndPoint', function() {
  it('Should check that the endpoint conforms with the schema', function(done) {
    makeTestEP(function(err, ep) {
      assert(schema.isValidEndPoint(ep));
      done();
    });
  });
  
  it('Should instantiate a new end point without problems', function(done) {
    mail2.tryMakeEndPoint("/tmp/newendpt.js", "newendpt", function(err, ep) {
      assert(!err);
      assert(ep instanceof mail2.EndPoint);
      assert.equal(ep.name, "newendpt");
      done();
    });
  });
  
  it('Should instantiate a new end point without problems, and reset it', function(done) {
    mail2.tryMakeAndResetEndPoint("/tmp/newendpt.js", "newendpt", function(err, ep) {
      assert(!err);
      assert(ep instanceof mail2.EndPoint);
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
    var C = mail2.srcDstPairUnion(A, B);
    assert(eq(C, [{src:'a', dst:'b'}, {src:'a', dst:'c'},
                  {src:'b', dst:'a'}, {src:'c', dst:'a'}]));
  });
  
  it('Compute the intersection', function() {
    var A = [{src:'a', dst:'b'}, {src:'a', dst:'c'}, {src:'b', dst:'a'}];
    var B = [{src:'a', dst:'c'}, {src:'c', dst:'a'}];
    var C = mail2.srcDstPairIntersection(A, B);
    assert(eq(C, [{src:'a', dst:'c'}]));
  });

  it('Compute the difference', function() {
    var A = [{src:'a', dst:'b'}, {src:'a', dst:'c'}, {src:'b', dst:'a'}];
    var B = [{src:'a', dst:'c'}, {src:'c', dst:'a'}];
    var C = mail2.srcDstPairDifference(A, B);
    assert(eq(C, [{src:'a', dst:'b'}, {src:'b', dst:'a'}]));
  });

  it('Handler', function() {
    makeTestEP(function(err, ep) {
      assert(ep.packetHandlers.length == 0);
      ep.addPacketHandler(function(ep, packet) {console.log('Got this packet: %j', p);});
      assert(ep.packetHandlers.length == 1);
      assert(typeof ep.packetHandlers[0] == 'function');
    });
  });
  
  it('Send a two packets, get the sorted src,dst pairs', function(done) {
    makeTestEP(function(err, ep) {
      ep.sendPacketAndReturn('b', 119, new Buffer(0), function(err, pa) {
        assert(!err);
        ep.sendPacketAndReturn('a', 119, new Buffer(0), function(err, pb) {
          assert(!err);
          ep.getSrcDstPairs(function(err, pairs) {
            assert(!err);
            assert(eq(pairs, [{src:'ep', dst:'a'}, {src:'ep', dst:'b'}]));
            ep.setLowerBound('a', 'ep', bigint.inc(bigint.zero()), function(err) {
              assert(!err);
              ep.getSrcDstPairs(function(err, pairs) {
                assert(!err);
                assert(eq(pairs, [{src:'ep', dst:'a'}, {src:'ep', dst:'b'}]));
                done();
              });
            });
          })
        });
      });
    });
  });

  it('Put a packet', function(done) {
    var p = {
      src: 'a',
      dst: 'b',
      label: 120,
      seqNumber: bigint.inc(bigint.zero()),
      data: new Buffer(0)
    };
    var q = {
      src: 'a',
      dst: 'b',
      label: 130,
      seqNumber: bigint.inc(p.seqNumber),
      data: new Buffer(0)
    };
    
    var r = {
      src: 'a',
      dst: 'ep',
      label: 130,
      seqNumber: bigint.inc(p.seqNumber),
      data: new Buffer(0)
    };

    var s = {
      src: 'a',
      dst: 'c',
      label: 131,
      seqNumber: bigint.inc(r.seqNumber),
      data: new Buffer(0)
    };

    var handledPacket = null;

    makeTestEP(function(err, ep) {
      ep.addPacketHandler(function(ep, packet) {
        handledPacket = packet;
      });
      ep.putPacket(p, function(err) {
        assert(!err);
        ep.getTotalPacketCount(function(err, count) {
          assert.equal(count, 1);
          ep.putPacket(p, function(err) {
            assert(!err);
            p.label = 130;
            ep.putPacket(p, function(err) {
              assert(err)
              ep.getTotalPacketCount(function(err, count) {
                assert.equal(count, 1);
                ep.putPacket(q, function(err) {
                  assert(!err);
                  ep.getTotalPacketCount(function(err, count) {
                    assert.equal(count, 2);
                    assert(!handledPacket);
                    ep.putPacket(r, function(err) {
                      assert(eq(r, handledPacket));
                      ep.getTotalPacketCount(function(err, count) {
                        assert.equal(count, 2);
                        ep.putPacket(s, function(err) {
                          ep.getLowerBounds(
                            [{src: 'a', dst: 'b'}, {src: 'a', dst: 'c'}],
                            function(err, lbs) {
                              assert(eq(lbs,
                                        [ { src: 'a', dst: 'b', lb: '0000000000000001' },
                                          { src: 'a', dst: 'c', lb: '0000000000000003' } ]));
                              ep.getUpperBounds(
                                [{src: 'a', dst: 'b'}, {src: 'a', dst: 'c'}],
                                function(err, ubs) {
                                  assert(eq(ubs,
                                            [ { src: 'a', dst: 'b', ub: '0000000000000003' },
                                              { src: 'a', dst: 'c', ub: '0000000000000004' } ]));
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
      });
    });    
  });
});

var endpoint = require('../endpoint.sqlite.js');
var assert = require('assert');
var bigint = require('../bigint.js');
var eq = require('deep-equal-ident');
var schema = require('../endpoint-schema.js');

function makeTestEP(cb) {
  endpoint.tryMakeAndResetEndpoint('/tmp/ep.db', 'ep', cb);
}


function insertPackets(db, packets, cb) {
  if (packets.length == 0) {
    cb(null);
  } else {
    var p = packets[0];
    db.get("INSERT INTO packets VALUES (?, ?, ?, ?, ?)",
           p[0], p[1], p[2], p[3], p[4],
           function(err) {
             if (err) {
               cb(err);
             } else {
               insertPackets(db, packets.slice(1), cb);
             }
           });
  }
}


describe('Endpoint', function() {
  it('Should check that the endpoint conforms with the schema', function(done) {
    makeTestEP(function(err, ep) {
      assert(schema.isValidEndpoint(ep));
      done();
    });
  });
  
  it('Should instantiate a new end point without problems', function(done) {
    endpoint.tryMakeEndpoint("/tmp/newendpt.js", "newendpt", function(err, ep) {
      assert(!err);
      assert(ep instanceof endpoint.Endpoint);
      assert.equal(ep.name, "newendpt");
      done();
    });
  });
  
  it('Should instantiate a new end point without problems, and reset it', function(done) {
    endpoint.tryMakeAndResetEndpoint("/tmp/newendpt.js", "newendpt", function(err, ep) {
      assert(!err);
      assert(ep instanceof endpoint.Endpoint);
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
            ep.updateLowerBound('ep', 'b', packet.seqNumber, function(err) {
              assert(!err);
              ep.getLowerBound('ep', 'b', function(err, lb) {
                assert(!err);
                assert.equal(lb, packet.seqNumber);
                ep.getTotalPacketCount(function(err, count) {
                  assert(!err);
                  assert(count == 1);
                  ep.updateLowerBound('ep', 'b', bigint.inc(packet.seqNumber), function(err) {
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
            ep.updateLowerBound('ep', 'b', bigint.inc(pb.seqNumber), function() {
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
    var C = endpoint.srcDstPairUnion(A, B);
    assert(eq(C, [{src:'a', dst:'b'}, {src:'a', dst:'c'},
                  {src:'b', dst:'a'}, {src:'c', dst:'a'}]));
  });
  
  it('Compute the intersection', function() {
    var A = [{src:'a', dst:'b'}, {src:'a', dst:'c'}, {src:'b', dst:'a'}];
    var B = [{src:'a', dst:'c'}, {src:'c', dst:'a'}];
    var C = endpoint.srcDstPairIntersection(A, B);
    assert(eq(C, [{src:'a', dst:'c'}]));
  });

  it('Compute the difference', function() {
    var A = [{src:'a', dst:'b'}, {src:'a', dst:'c'}, {src:'b', dst:'a'}];
    var B = [{src:'a', dst:'c'}, {src:'c', dst:'a'}];
    var C = endpoint.srcDstPairDifference(A, B);
    assert(eq(C, [{src:'a', dst:'b'}, {src:'b', dst:'a'}]));
  });

  it('Handler', function() {
    makeTestEP(function(err, ep) {
      assert(ep.packetHandlers.length == 1);
      ep.addPacketHandler(function(endpoint, packet) {console.log('Got this packet: %j', p);});
      assert(ep.packetHandlers.length == 2);
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
            ep.updateLowerBound('a', 'ep', bigint.inc(bigint.zero()), function(err) {
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
      ep.addPacketHandler(function(endpoint, packet) {
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
                                        [ '0000000000000001', '0000000000000003' ]));
                              ep.getUpperBounds(
                                [{src: 'a', dst: 'b'}, {src: 'a', dst: 'c'}],
                                function(err, ubs) {
                                  assert(eq(ubs,
                                            [ '0000000000000003' ,'0000000000000004' ]));
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

  it('updatelowerbounds', function(done) {
    makeTestEP(function(err, ep) {
      assert(!err);
      ep.updateLowerBounds([
        {src:"a", dst:"b", lb:"0004"},
        {src:"a", dst:"c", lb:"0005"}
      ], function(err, lbs) {
        assert(!err);
        assert(eq(lbs, ["0004", "0005"]));
        ep.updateLowerBounds([
          {src:"a", dst:"b", lb:"0003"},
          {src:"a", dst:"c", lb:"0006"}
        ], function(err, lbs) {
          assert(!err);
          assert(eq(lbs, ["0004", "0006"]));
          done();
        });
      });
    });
  });

  /*

sqlite> select * from packets;
boat553910775bfc1709601c6aa9|boxfcc2de3178ef|0000014e1b4b8d6e|129|��type�sh�script�/tmp/run.sh�reqCode�558802f599080b35399d709f
boat553910775bfc1709601c6aa9|boxfcc2de3178ef|0000014e1b4b8d6f|129|��type�sh�script
boat553910775bfc1709601c6aa9|boxfcc2de3178ef|0000014e1b4b8d70|129|��type�sh�script
boat553910775bfc1709601c6aa9|boxfcc2de3178ef|0000014e1b4b8d71|129|��type�sh�script
sqlite> select * from lowerBounds;
boat553910775bfc1709601c6aa9|boxfcc2de3178ef|0000014e1b4948c6
boxfcc2de3178ef|boat553910775bfc1709601c6aa9|0000014e1ad6b2b7
sqlite> 
    
    */

  it('updatelowerbounds2', function(done) {
    makeTestEP(function(err, ep) {
      ep.updateLowerBound(
        "boat553910775bfc1709601c6aa9", "boxfcc2de3178ef",
        "0000014e1b4948c4", function(err) {

          // Adding these packets just like this is going to
          // put the table in a corrupt state, because the lowest packet number
          // is greater than the value in the lower bound table.
          insertPackets(
            ep.db,
            [["boat553910775bfc1709601c6aa9",
              "boxfcc2de3178ef", "0000014e1b4b8d6e", 129,
              new Buffer(0)],
             ["boat553910775bfc1709601c6aa9",
              "boxfcc2de3178ef", "0000014e1b4b8d6f", 129,
              new Buffer(0)],
             ["boat553910775bfc1709601c6aa9",
              "boxfcc2de3178ef", "0000014e1b4b8d70", 129,
              new Buffer(0)],
             ["boat553910775bfc1709601c6aa9",
              "boxfcc2de3178ef", "0000014e1b4b8d71", 129,
              new Buffer(0)]],
            function(err) {
              assert(!err);
              ep.getLowerBound("boat553910775bfc1709601c6aa9",
                               "boxfcc2de3178ef", function(err, lb) {
                var lb0 = "0000014e1b4b8d6e";
                var lb1 = "0000014e1b4948c6";
                assert(lb1 < lb0);
                assert(lb == lb0);
                ep.updateLowerBounds([
                  {"src":"boat553910775bfc1709601c6aa9",
                   "dst":"boxfcc2de3178ef","lb":"0000014e1b4948c6"},
                  {"src":"boxfcc2de3178ef",
                   "dst":"boat553910775bfc1709601c6aa9","lb":"0000014e1ad6b2b2"}
                ], function(err, lbs) {
                  assert(lbs[0] == lb0);
                  assert(!err);
                  console.log('LBS = ');
                  console.log(lbs);
                  done();
                });
              });
            });
        });
    });
  });

  it('Should produce a correct seq number', function(done) {
    makeTestEP(function(err, ep) {
      ep.updateLowerBound(
        "boat553910775bfc1709601c6aa9", "boxfcc2de3178ef",
        "0000000000000001", function(err) {
          assert(!err);
          ep.getNextSeqNumber(
            "boat553910775bfc1709601c6aa9",
            "boxfcc2de3178ef", function(err, sn) {
              assert(!err);
              assert(sn == "0000000000000001");
              done();
            });
        });
    });
  });
});

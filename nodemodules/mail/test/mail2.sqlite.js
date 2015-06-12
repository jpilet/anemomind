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
});

var mb = require('../mail2.sqlite.js');
var assert = require('assert');

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
      console.log(err);
      assert(!err);
      assert(ep instanceof mb.EndPoint);
      assert.equal(ep.name, "newendpt");
      done();
    });
  });
});

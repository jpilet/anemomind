var DelayedCall = require('../components/DelayedCall.js');
var assert = require('assert');

describe('DelayedCall', function() {
  it('Should call a function with delayed', function(done) {
    var performed = false;
    var dc = new DelayedCall(function() {performed = true;});
    dc.callDelayed(150);
    setTimeout(function() {
      assert(!performed);
      setTimeout(function() {
        assert(performed);
        done();
      }, 100);
    }, 100);
  });
});

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

  it('Should reschedule a delayed call later', function(done) {
    var performed = false;
    var dc = new DelayedCall(function() {performed = true;});
    dc.callDelayed(100); // At time 100
    setTimeout(function() { // At time 50
      assert(!performed);
      dc.callDelayed(200); // At time 50 + 200 = 250
      setTimeout(function() { // At time 50 + 100 = 150
        assert(!performed);
        setTimeout(function() { // At time 300
          assert(performed);
          done();
        }, 150);
      }, 100);
    }, 50);
  });
});

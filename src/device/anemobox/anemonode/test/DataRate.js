var dr = require('../components/DataRate.js');
var assert = require('assert');

function produceTimedCalls(count, period, call, cb) {
  if (count <= 0) {
    cb();
  } else {
    call();
    setTimeout(function() {
      produceTimedCalls(count-1, period, call, cb);
    }, period);
  }
}

describe('DataRate', function() {
  it('Rate estimator', function() {
    // Posting 60 bytes every 0.1 seconds. 
    // That is a rate of 600 bytes per second
    var r = new dr.RateEstimator(10.0);
    for (var i = 0; i < 10000; i++) {
      r.add(0.1*i, 60);
    }
    console.log("The rate is " + r.estimate());
    assert(Math.abs(r.estimate() - 600) < 10);
  });

  it('ByteRateLimiting', function(done) {
    this.timeout(10000);
    var acc = dr.limitRateAcceptor(
      100, 0.5); // 100 bytes per second, 10 per hundreth

    var counter = 0;

    var n = 100;
    produceTimedCalls(n, 33, function() {
      if (acc("ABCDEFGHIJ")) {
        counter++;
      }
    }, function() {
      // Because we are sending at a rate of about 300 bytes per second,
      // and the max rate is 100, we expect about a third to be kept.
      console.log("Got %d of %d", counter, n);
      assert(Math.abs(counter - 0.33*n) <= 20); 
      done();
    });
  });
});

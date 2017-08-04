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
  it('ByteRateLimiting', function(done) {
    this.timeout(10000);
    var acc = new dr.NaiveRateLimiter(
      100, 0.5); // 100 bytes per second, 10 per hundreth

    var counter = 0;

    var n = 100;
    produceTimedCalls(n, 33, function() {
      if (acc.accept("ABCDEFGHIJ".length)) {
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

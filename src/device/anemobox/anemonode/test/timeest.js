var timeest = require('../components/timeest');
var assert = require('assert');
var should = require('should');

function MockChannel(sysSeconds, gpsSeconds) {
  assert(sysSeconds.length == gpsSeconds.length);
  this.sysSeconds = sysSeconds;
  this.gpsSeconds = gpsSeconds;
  this.offset = new Date(1467881037966);
}

MockChannel.prototype.length = function() {
  return this.sysSeconds.length;
}

MockChannel.prototype.secondsToTime = function(secs, index) {
  return new Date(this.offset.getTime() + 1000*secs[index]);
}

MockChannel.prototype.time = function(index) {
  return this.secondsToTime(this.sysSeconds, index);
}

MockChannel.prototype.value = function(index) {
  return this.secondsToTime(this.gpsSeconds, index);
}

var sysSeconds = [0, 1, 2, 3, 5, 6, 9, 10, 11];
sysSeconds.sort(function(a, b) {return a < b;})

var trueGpsSeconds = sysSeconds.map(function(x) {return x + 3000;});
var corruptedGpsSeconds = trueGpsSeconds.slice(0);
corruptedGpsSeconds[4] = -1233;
corruptedGpsSeconds[5] = 99;

var src = new MockChannel(sysSeconds, corruptedGpsSeconds);

describe('timeest', function() {
  it('median-delta-time', function() {
    var expectedOffsetMilliseconds = 3000000;
    assert(timeest.medianDeltaTime(src, 2) == expectedOffsetMilliseconds);
    assert(timeest.medianDeltaTime(src, 4) == expectedOffsetMilliseconds);
    assert(timeest.medianDeltaTime(src, 5) == expectedOffsetMilliseconds);
    assert(timeest.medianDeltaTime(src, sysSeconds.length-1) == expectedOffsetMilliseconds);
    assert(timeest.medianDeltaTime(src, 30) == expectedOffsetMilliseconds);

    var currentSystemTime = new Date(1467881037966 + 11*1000);

    var expectedTime = new Date(1467881037966 + (3000 + 11)*1000);

    assert(timeest.estimateTime(currentSystemTime, src, 4).getTime()
           == expectedTime.getTime());

    // Run same query again to check that the memoized version works
    assert(timeest.estimateTime(currentSystemTime, src, 4).getTime() 
           == expectedTime.getTime());
    assert(timeest.estimateTime(currentSystemTime, src, 4).getTime() 
           == expectedTime.getTime());
  });

  it ('should return a ref date published to the Dispatcher', function(done) {
    var ref = new Date("Thu Jul 6 2016 13:45:34 GMT+0200 (CEST)");

    var anemonode = require('../build/Release/anemonode');
    anemonode.dispatcher.values.dateTime.setValue("test", ref);

    var estimated = timeest.now();

    (estimated.getTime()).should.be.approximately(ref.getTime(), 2);

    var delay = 100;
    setTimeout(function() {
      var estimated = timeest.now();
      // 100 ms should have passed.
      (estimated.getTime()).should.be.approximately(ref.getTime() + delay, 20);
      done();
    }, 100);

  });
});


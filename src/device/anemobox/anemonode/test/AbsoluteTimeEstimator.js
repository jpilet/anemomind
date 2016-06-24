var assert = require('assert');
var AbsoluteTimeEstimator = require('../components/AbsoluteTimeEstimator.js');

describe('AbsoluteTimeEstimator', function() {
  it('ate', function() {
    var systemTime = [0, 1000, 2000, 4000, 9000, 12000, 13000];
    var offset = 60000;
    var trueTime = systemTime.map(function(x) {
      return x + offset;
    });
    var timeWithOutliers = trueTime.map(function(index, x) {
      if (index == 3) {
        return 1234343;
      } else if (index == 4) {
        return 55005550;
      }
      return x;
    });

    var est = new AbsoluteTimeEstimator(30);
    for (var i = 0; i < 7; i++) {
      est.addTimePair(new Date(systemTime[i]), new Date(timeWithOutliers[i]));
    }
    
    var estimatedTimes = systemTimes.map(function(x) {
      return est.estimateCurrentTime(new Date(x));
    });

    console.log("True times");
    console.log(trueTimes);
    console.log("Estimated times");
    console.log(estimatedTimes);
  });
});


var mail2 = require('./mail2.sqlite.js');
var bigint = require('./bigint.js');
var common = require('./common.js');
var assert = require('assert');

function disp(a, b, cb) {
  a.disp(function(err) {
    b.disp(cb);
  });
}

function transferPacketsSub(pair, fromIndex, toIndex, from, to, putPacket, cb) {
  if (fromIndex == toIndex) {
    cb();
  } else {
    from.getPacket(pair.src, pair.dst, fromIndex, function(err, packet) {
      if (err) {
        cb(err);
      } else if (!packet) {
        cb(new Error("Missing packet"));
      } else {
        putPacket(to, packet, function(err) {
          if (err) {
            cb(err);
          } else {
            transferPacketsSub(pair, bigint.inc(fromIndex), toIndex, from, to, putPacket, cb);
          }
        });
      }
    });
  }
}

var zero = bigint.zero();
function transferPackets(pair, fromIndex, toIndex, from, to, cb) {
  if (toIndex == zero || fromIndex == toIndex) {
    cb();
  } else if (fromIndex == 0) {
    to.getLowerBound(pair.src, pair.dst, function(err, fromIndex2) {
      if (err) {
        cb(err);
      } else {
        transferPackets(pair, from2, toIndex, from, to, cb);
      }
    });
  } else {
    var putPacket = function(dst, packet, cb) {
      dst.putPacket(packet, cb);
    };
    transferPacketsSub(pair, fromIndex, toIndex, from, to, putPacket, cb);
  }
}

function synchronizePackets(pair, a, b, cb) {
  a.getUpperBound(pair.src, pair.dst, function(err, ubA) {
    b.getUpperBound(pair.src, pair.dst, function(err, ubB) {
      if (ubA > ubB) {
        transferPackets(pair, ubB, ubA, a, b, cb);
      } else if (ubA < ubB) {
        transferPackets(pair, ubA, ubB, b, a, cb);
      } else {
        cb();
      }
    });
  });
}

function getCommonSrcDstPairs(a, b, cb) {
  a.getSrcDstPairs(function(err, aPairs) {
    if (err) {
      cb(err);
    } else {
      b.getSrcDstPairs(function(err, bPairs) {
        if (err) {
          cb(err);
        } else {
          var abPairs = mail2.srcDstPairUnion(aPairs, bPairs);
          if (aPairs.isLeaf) {
            abPairs = mail2.filterByName(abPairs, a.name);
          }
          if (bPairs.isLeaf) {
            abPairs = mail2.filterByName(abPairs, b.name);
          }
          cb(null, abPairs);
        }
      });
    }
  });
}

function runSyncJob(job, putPacket, cb) {
  transferPacketsSub(job.pair, job.fromIndex,
                     job.toIndex, job.from, job.to, putPacket, cb);
}


function countPackets(jobs) {
  var counter = 0;
  for (var i = 0; i < jobs.length; i++) {
    var job = jobs[i];
    counter += Math.abs(bigint.diff(job.fromIndex, job.toIndex));
  }
  return counter;
}

function runSyncJobsSub(jobs, putPacket, cb) {
  if (jobs.length == 0) {
    cb();
  } else {
    runSyncJob(jobs[0], putPacket, function(err) {
      if (err) {
        cb(err);
      } else {
        runSyncJobsSub(jobs.slice(1), putPacket, cb);
      }
    });
  }
}

function runSyncJobs(jobs, cb, cbProgress) {
  var reportProgress = cbProgress || function() {};
  var counter = 0;
  var totalCount = countPackets(jobs);
  
  var putPacket = function(dst, packet, cb) {
    dst.putPacket(packet, function(err) {
      if (!err) {
        counter++;
        reportProgress(counter, totalCount);
      }
      cb(err);
    });
  }

  runSyncJobsSub(jobs, putPacket, cb);
}

function calcFromIndex(lb, aub, bub) {
  if (aub == bigint.zero() || bub == bigint.zero()) {
    return lb;
  } else if (aub < bub) {
    return aub;
  }
  return bub;
}

function filterSyncJob(job) {
  var syncBack = false;
  if (!syncBack && job.to.name == job.pair.src) {
    job.fromIndex = job.toIndex;
  }
  return job;
}

function makeSyncJob(pair, lb, aub, bub, a, b) {
  var fromIndex = calcFromIndex(lb, aub, bub);
  if (aub < bub) {
    return filterSyncJob(
      {pair: pair, fromIndex: fromIndex, toIndex: bub, from: b, to: a});
  } else {
    return filterSyncJob(
      {pair: pair, fromIndex: fromIndex, toIndex: aub, from: a, to: b});
  }
}

function makeSyncJobs(pairs, lbs, aubs, bubs, a, b) {
  var count = pairs.length;
  assert(count == aubs.length);
  assert(count == bubs.length);
  assert(count == lbs.length);
  var jobs = new Array(count);
  for (var i = 0; i < count; i++) {
    jobs[i] = makeSyncJob(pairs[i], lbs[i], aubs[i], bubs[i], a, b);
  }
  return jobs;
}

//// Synchronize all lower bounds, and obtain the new lower bounds.
//// cb is cb(err, lbs) where lbs are the synchronized lower bounds
function synchronizeAllLowerBounds(pairs, a, b, cb) {
  var results = new common.ResultArray(2, function(err, lbs) {
    if (err) {
      cb(err);
    } else {
      var albs = lbs[0];
      var blbs = lbs[1];
      var setResults = new common.ResultArray(pairs.length, cb);
      for (var i = 0; i < pairs.length; i++) {
        var set = setResults.makeSetter(i);
        var pair = pairs[i];
        var alb = albs[i];
        var blb = blbs[i];
        if (alb < blb) {
          a.setLowerBound(pair.src, pair.dst, blb, common.makeValuePasser(blb, set));
        } else if (alb > blb) {
          b.setLowerBound(pair.src, pair.dst, alb, common.makeValuePasser(alb, set));
        } else { // same
          assert(alb == blb);
          set(null, alb);
        }
      }
    }
  });
  a.getLowerBounds(pairs, results.makeSetter(0));
  b.getLowerBounds(pairs, results.makeSetter(1));
}



function synchronizeAllPackets(pairs, lbs, a, b, cb, cbProgress) {
  var reportProgress = cbProgress || function(counter, total) {
    console.log('Synchronized %d of %d packets between %s and %s', counter, total, a.name, b.name);
  };
  var ubResults = new common.ResultArray(2, function(err, ubs) {
    if (err) {
      cb(err);
    } else {
      var jobs = makeSyncJobs(pairs, lbs, ubs[0], ubs[1], a, b);
      runSyncJobs(jobs, cb, reportProgress);
    }
  });
  a.getUpperBounds(pairs, ubResults.makeSetter(0));
  b.getUpperBounds(pairs, ubResults.makeSetter(1));
}

function synchronize(a, b, cb, cbProgress) {
  if (a.name == b.name) {
    cb(new Error('The end points must be different'));
  } else {
    getCommonSrcDstPairs(a, b, function(err, pairs) {
      if (err) {
        cb(err);
      } else {
        synchronizeAllLowerBounds(pairs, a, b, function(err, lbs) {
          if (err) {
            cb(err);
          } else {
            synchronizeAllPackets(pairs, lbs, a, b, cb, cbProgress);
          }
        });
      }
    });
  }
}

module.exports.synchronize = synchronize;
module.exports.disp = disp;

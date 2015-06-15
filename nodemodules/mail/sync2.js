var mail2 = require('./mail2.sqlite.js');
var bigint = require('./bigint.js');

function disp(a, b, cb) {
  a.disp(function(err) {
    b.disp(cb);
  });
}

function synchronizeLowerBounds(pair, a, b, cb) {
  a.getLowerBound(pair.src, pair.dst, function(err, lbA) {
    if (err) {
      cb(err);
    } else {
      b.getLowerBound(pair.src, pair.dst, function(err, lbB) {
        if (err) {
          cb(err);
        } else {
          if (lbA < lbB) {
            a.setLowerBound(pair.src, pair.dst, lbB, cb);
          } else if (lbA > lbB) {
            b.setLowerBound(pair.src, pair.dst, lbA, cb);
          } else {
            cb();
          }
        }
      });
    }
  });
}

function transferPacketsSub(pair, fromIndex, toIndex, from, to, cb) {
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
            transferPacketsSub(pair, bigint.inc(fromIndex), toIndex, from, to, cb);
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
    transferPacketsSub(pair, fromIndex, toIndex, from, to, cb);
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

function synchronizePair(pair, a, b, cb) {
  synchronizeLowerBounds(pair, a, b, function(err) {
    if (err) {
      cb(err);
    } else {
      synchronizePackets(pair, a, b, cb);
    }
  });
}

function synchronizePairs(pairs, a, b, cb) {
  if (pairs.length == 0) {
    cb();
  } else {
    var pair = pairs[0];
    synchronizePair(pair, a, b, function(err) {
      if (err) {
        cb(err);
      } else {
        synchronizePairs(pairs.slice(1), a, b, cb);
      }
    });
  }
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

function synchronizeAllLowerBounds(pairs, a, b, cb) {
  var results = new common.ResultsArray(2, function(err, lbs) {
    if (err) {
      cb(err);
    } else {
      var albs = results[0];
      var blbs = results[1];
      var setResults = new common.ResultArray(pairs.length, function(err) {
        if (err) {
          cb(err);
        } else {
          cb(null, albs, blbs);
        }
      });
      for (var i = 0; i < pairs.length; i++) {
        var set = setResults.makeSetter(i);
        var pair = pairs[i];
        var alb = albs[i];
        var blb = blbs[i];
        if (alb < blb) {
          albs[i] = blb;
          a.setLowerBound(pair.src, pair.dst, blb, set);
        } else if (alb > blb) {
          blbs[i] = alb;
          b.setLowerBound(pair.src, pair.dst, alb, set);
        } else {
          set();
        }
      }
    }
  });
  a.getLowerBounds(pairs, results.makeSetter(0));
  b.getLowerBounds(pairs, results.makeSetter(1));
}

function calcTotalPacketCountToTransfer(ubs) {
  assert(ubs.length == 2);
  var a = ubs[0];
  var b = ubs[1];
  assert(a.length == b.length);
  var count = 0;
  for (var i = 0; i < a.length; i++) {
    count += abs(bigint.diff(a[i], b[i]));
  }
  return count;
}

function synchronizeAllPacketsSub(i, pairs, aubs, bubs, a, b, putPacketSub, cb) {
  assert(pairs.length == aubs.length);
  assert(pairs.length == bubs.length);
  if (i == pairs.length) {
    cb();
  } else {

    // Next iteration
    var next = function(err) {
      if (err) {
        cb(err);
      } else {
        cb(i+1, pairs, aubs, bubs, a, b, putPacketSub, cb);
      }
    }
    
    var pair = pairs[i];
    var aub = aubs[i];
    var bub = bubs[i];
    if (aub > bub) {
      transferPackets(pair, bub, aub, a, b, putPacketSub, next);
    } else if (aub < bub) {
      transferPackets(pair, aub, bub, b, a, putPacketSub, next);
    } else {
      next();
    }
  }
}

function synchronizeAllPackets(pairs, a, b, cb, cbProgress) {
  var reportProgress = cbProgress || function() {};
  var ubResults = new common.ResultArray(2, function(err, ubs) {
    if (err) {
      cb(err);
    } else {
      // Putting packets and reporting progress.
      var totalPacketCount = calcTotalPacketCountToTransfer(ubs);
      var putCounter = 0;
      var putPacketSub = function(dst, packet, cb) {
        dst.putPacket(packet, function(err) {
          if (!err) {
            putCounter++;
            reportProgress(putCounter, totalPacketCount);
          }
          cb(err);
        });
      }
      
      // Actual packet transfer.
      synchronizeAllPacketsSub(0, pairs, aubs, bubs, a, b, putPacketSub, cb);
    }
  });
  a.getUpperBounds(pairs, ubResults.makeSetter(0));
  b.getUpperBounds(pairs, ubResults.makeSetter(1));
}

function synchronize(a, b, cb) {
  if (a.name == b.name) {
    cb(new Error('The end points must be different'));
  } else {
    getCommonSrcDstPairs(a, b, function(err, pairs) {
      if (err) {
        cb(err);
      } else {
        synchronizePairs(pairs, a, b, cb);
      }
    });
  }
}

module.exports.synchronize = synchronize;
module.exports.disp = disp;

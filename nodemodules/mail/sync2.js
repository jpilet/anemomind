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
      if (ubA < ubB) {
        transferPackets(pair, ubA, ubB, a, b, cb);
      } else if (ubA > ubB) {
        transferPackets(pair, ubB, ubA, b, a, cb);
      } else {
        cb();
      }
    });
  });
}

function synchronizePair(pair, a, b, cb) {
  synchronizeLowerBounds(pair, a, b, function(err) {
  });
}

function synchronizePairs(pairs, a, b, cb) {
  if (pairs.length == 0) {
    cb();
  } else {
    synchronizePair(pairs[0], a, b, function(err, cb) {
      if (err) {
        cb(err);
      } else {
        synchronizePairs(pairs.slice(1), a, b, cb);
      }
    });
  }
}

function synchronize(a, b, cb) {
  getCommonSrcDstPairs(a, b, function(err, pairs) {
    if (err) {
      cb(err);
    } else {
      synchronizePairs(pairs, a, b, cb);
    }
  });
}

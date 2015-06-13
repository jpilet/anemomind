var mail2 = require('./mail2.sqlite.js');
var bigint = require('./bigint.js');

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
  console.log('Transfer packets from ' + from.name + ' to ' + to.name);
  if (fromIndex == toIndex) {
    cb();
  } else {
    from.getPacket(pair.src, pair.dst, fromIndex, function(err, packet) {
      console.log('Get packet: %j', packet);
      if (err) {
        cb(err);
      } else if (!packet) {
        cb(new Error("Missing packet"));
      } else {
        to.putPacket(packet, function(err) {
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
  console.log('Transfer the packets from ' + from.name + ' to ' + to.name);
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
  console.log('synchronizePackets');
  console.log('Aname = ', a.name);
  console.log('Bname = ', b.name);
  a.getUpperBound(pair.src, pair.dst, function(err, ubA) {
    b.getUpperBound(pair.src, pair.dst, function(err, ubB) {
      console.log('ubA = ' + ubA);
      console.log('ubB = ' + ubB);
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
  console.log('synchronizePair');
  console.log('Aname = ', a.name);
  console.log('Bname = ', b.name);
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

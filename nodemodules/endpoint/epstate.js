var ep = require('./endpoint.sqlite.js');
var assert = require('assert');


function before(a, b) {
  assert(a instanceof Array);
  assert(b instanceof Array);
  assert(a.length == b.length);
  for (var i = 0; i < a.length; i++) {
    if (a[i] < b[i]) {
      return true;
    } else if (a[i] > b[i]) {
      return false;
    }
  }
  return false;
}

function arrEq(a, b) {
  return !before(a, b) && !before(b, a);
}

function insertSimilar(dst, key, value) {
  for (var i = 0; i < dst.length; i++) {
    if (arrEq(dst[i].key, key)) {
      dst[i].values.push(value);
      return;
    }
  }
  dst.push({key: key, values: [value]});
}

function groupSimilar(data, keyFn) {
  var dst = [];
  for (var i = 0; i < data.length; i++) {
    var x = data[i];
    insertSimilar(dst, keyFn(x), x);
  }
  return dst;
}

function minv(a, b) {
  return a < b? a : b;
}

function maxv(a, b) {
  return a < b? b : a;
}

function getSeqNumber(x) {
  return x.seqNumber;
}

function summarizePackets(packets) {
  var groups = groupSimilar(packets, function(packet) {
    return [packet.src, packet.dst, packet.label];
  });
  var p = packets[0];
  return groups.map(function(group) {
    var seqNums = group.values.map(getSeqNumber);
    return {
      src: p.src,
      dst: p.dst,
      label: p.label,
      count: group.values.length,
      minSeqNumber: seqNums.reduce(minv),
      maxSeqNumber: seqNums.reduce(maxv)
    };
  });
}

function getAllEndpointData(endpoint, cb) {
  ep.getAllFromTable(endpoint.db, 'packets', function(err, packets) {
    if (err) {
      cb(err);
    } else {
      ep.getAllFromTable(endpoint.db, 'lowerBounds', function(err, lowerBounds) {
        if (err) {
          cb(err);
        } else {
          cb(null, {
            packets: packets,
            lowerBounds: lowerBounds
          });
        }
      });
    }
  });
}

function summarizeEndpointData(data) {
  assert(data.packets);
  assert(data.lowerBounds);
  return {
    packets: summarizePackets(data.packets),
    lowerBounds: data.lowerBounds
  };
}

module.exports.getAllEndpointData = getAllEndpointData;
module.exports.summarizeEndpointData = summarizeEndpointData;

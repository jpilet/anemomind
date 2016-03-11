var ep = require('./endpoint.sqlite.js');
var assert = require('assert');
var common = require('./common.js');
var mkdirp = require('mkdirp');
var Path = require('path');
var fs = require('fs');
var Q = require('q');
var rmdir = require('rmdir');
var exec = require('child_process').exec;

/*

  HOW TO USE
  
  On the sender side: Send large packets using largepacket.sendPacket

  On the receiver side: call endpoint.addPacketHandler(largepacket.makeLargePacketHandler( ... ))

*/

function validSendPacketData(dst, localEndpoint, label, data, settings, cb) {
  return localEndpoint.sendSimplePacketAndReturn // To acquire sequence number
    && (typeof label == 'number')
    && (data instanceof Buffer)
    && (typeof cb == 'function');
}

// How many bytes the fields occupy
var labelLength = 4;
var countLength = 4;

function splitBuffer(data, maxPacketSize) {
  assert(data instanceof Buffer);
  packetCount = Math.ceil(data.length/maxPacketSize);
  var dst = new Array(packetCount);
  for (var i = 0; i < packetCount; i++) {
    var fromIndex = i*maxPacketSize;
    var toIndex = Math.min(fromIndex + maxPacketSize, data.length);
    dst[i] = data.slice(fromIndex, toIndex);
  }
  return dst;
}


// The first packet holds this:
//  * The label of the packets to be assembled.
//  * The total number of packets: This is used, so that we know when we 
//    have received all packets.
//  * The first chunk of data
// The sequence number of the first packet is used to identify the entire 
// group of packets that make up the large packet.
function encodeFirstPacket(label, totalPacketCount) {
  var dst = new Buffer(labelLength + countLength);
  dst.writeUInt32LE(label, 0);
  dst.writeUInt32LE(totalPacketCount, labelLength);
  return dst;
}

function decodeFirstPacket(data0) {
  if (data0.length < labelLength + countLength) {
    return null;
  } else {
    var label = data0.readUInt32LE(0);
    var count = data0.readUInt32LE(labelLength);
    return {label: label, count: count};
  }
}

function stringSizeInBuffer(str) {
  return 4 + str.length;
}

function writeStringToBuffer(buffer, str, pos) {
  assert(buffer instanceof Buffer);
  assert(typeof str == 'string');
  buffer.writeUInt32LE(str.length, pos);
  buffer.write(str, pos + 4);
}

function readStringFromBuffer(buffer, pos) {
  assert(buffer instanceof Buffer);
  var len = buffer.readUInt32LE(pos);
  var offset = pos + 4;
  var end = offset + len;
  var s = buffer.toString('ascii', offset, end);
  assert(end - pos == stringSizeInBuffer(s));
  return {s: s, end: end}
}

function encodeRemainingPacket(seqNumber, data) {
  assert(typeof seqNumber == 'string');
  var seqNumberLength = stringSizeInBuffer(seqNumber);
  var dst = new Buffer(seqNumberLength + data.length);
  writeStringToBuffer(dst, seqNumber, 0);
  data.copy(dst, seqNumberLength);
  return dst;
}

function decodeRemainingPacket(data0) {
  try {
    var seqNumberAndOffset = readStringFromBuffer(data0, 0);
    var seqNumber = seqNumberAndOffset.s;
    var data = data0.slice(seqNumberAndOffset.end, data0.length);
    return {seqNumber: seqNumber, data: data};
  } catch (e) {
    return null;
  }
}

function makeGenerator(dst, label, n) {
  return function(sent, element) {
    // The first element in the array over which we iterate is null.
    assert((sent.length == 0) == (element == null));

    if (sent.length == 0) {
      return { // Return the first packet in the sequence
        dst: dst,
        data: encodeFirstPacket(label, n),
        label: common.firstPacket
      };
    } else {
      // Use the seqNumber of the first packet
      // to group the packets
      var seqNumber = sent[0].seqNumber; 
      return {
        dst: dst,
        data: encodeRemainingPacket(seqNumber, element),
        label: common.remainingPacket
      };
    }
  };
}

function sendPacket(localEndpoint, dst, label, data, settings, cb) {
  if (validSendPacketData(dst, localEndpoint, label, data, settings, cb)) {
    var packets = splitBuffer(data, settings.mtu);
    var array = [null].concat(packets);
    localEndpoint.sendSimplePacketBatch(
      array, makeGenerator(dst, label, packets.length), 
      function(err, sent) {
        if (err) {
          cb(err);
        } else {
          cb(null, {
            src: localEndpoint.name,
            dst: dst,
            label: label,
            seqNumber: sent[0].seqNumber,
            data: data
          });
        }
      });
  } else if (typeof cb == 'function') {
    cb(new Error(["Invalid data to largepacket.sendPacket: ", 
                  [dst, localEndpoint, label, data, settings, cb]]));
  } else {
    console.log('Invalid callback passed to largepacket.sendPacket: ' + cb);
  }
}

function preparePath(partsPath, src, seqNumber, cb) {
  var p = Path.join(partsPath, src + '_' + seqNumber + '');
  mkdirp(p, function(err) {
    cb(err, p);
  });
}


function handleFirstPacket(endpoint, packet) {
  ep.withTransaction(endpoint.db, function(T, cb) {
    ep.storePacket(T, packet, cb);
  }, function(err) {
    if (err) {
      console.log('ERROR in handleFirstPacket:');
      console.log(err);
    }
  });
}

function getFirstPacket(T, remainingPacket, cb) {
  var decodedPacket = decodeRemainingPacket(remainingPacket.data);
  T.get(
    'SELECT * FROM packets WHERE src = ? AND dst = ? AND seqNumber = ?', 
    remainingPacket.src, remainingPacket.dst, decodedPacket.seqNumber, cb);
}

function countRemainingPacketsInRange(T, src, dst, fromSeq, toSeq, cb) {
  T.get('SELECT count(*) FROM packets WHERE src = ? AND dst = ?'
        + ' AND ? <= seqNumber AND seqNumber <= ? AND label = ?',
        src, dst, fromSeq, toSeq, common.remainingPacket,
        function(err, row) {
          if (err) {
            cb(err);
          } else {
            cb(null, row['count(*)']);
          }
        });
}

function isComplete(T, remainingPacket, cb) {
  getFirstPacket(T, remainingPacket, function(err, first) {
    if (err) {
      cb(err);
    } else {
      var decodedFirst = decodeFirstPacket(first.data);
      var src = remainingPacket.src;
      var dst = remainingPacket.dst;
      var fromSeq = first.seqNumber;
      var toSeq = remainingPacket.seqNumber;
      countRemainingPacketsInRange(
        T, src, dst, fromSeq, toSeq,
        function(err, n) {
          if (err) {
            cb(err);
          } else if (n > decodedFirst.count) {
            cb(null, new Error('Too many packets'));
          } else if (n < decodedFirst.count) {
            cb();
          } else {
            cb(null, decodedFirst, first);
          }
        });
    }
  });
}

function concatParts(packets) {
  var unpacked = packets.map(function(p) {
    return decodeRemainingPacket(p.data).data;
  });

  for (var i = 0; i < unpacked.length; i++) {
    if (!(unpacked[i] instanceof Buffer)) {
      return null;
    }
  }

  var totalSize = unpacked.map(function(p) {return p.length;})
      .reduce(function(a, b) {return a + b;});

  var data = new Buffer(totalSize);
  var offset = 0;
  for (var i = 0; i < unpacked.length; i++) {
    var p = unpacked[i];
    p.copy(data, offset);
    offset += p.length;
  }
  assert(offset == totalSize);
  return data;
}

function assemblePacket(T, endpoint, first, decodedFirst, lastPacket, cb) {
  var src = lastPacket.src;
  var dst = lastPacket.dst;
  T.all('SELECT * FROM packets WHERE src = ? AND dst = ? AND ? < seqNumber'
        +' AND seqNumber <= ? AND label = ? ORDER BY seqNumber',
        src, dst, first.seqNumber,
        lastPacket.seqNumber, common.remainingPacket, function(err, packets) {
          var data = concatParts(packets);
          if (data == null) {
            cb(new Error('Failed to concatenate parts'));
          } else {
            endpoint.callPacketHandlers({
              src: src,
              dst: dst,
              data: data,
              label: decodedFirst.label,
              seqNumber: decodedFirst.seqNumber
            });
            cb();
          }
        });
}

function removePacketsInRange(T, src, dst, fromIncl, toIncl, cb) {
  T.run(
    'DELETE FROM packets WHERE src = ? AND dst = ? AND ? <= seqNumber AND seqNumber <= ?',
    src, dst, fromIncl, toIncl, cb);
}

function attemptToAssemblePacket(T, endpoint, lastPacket, cb) {
  return isComplete(T, lastPacket, function(err, decodedFirst, first) {
    if (decodedFirst) {
      assemblePacket(T, endpoint, first, decodedFirst, lastPacket, function(err, finalPacket) {
        if (err) {
          cb(err);
        } else {
          removePacketsInRange(
            T, lastPacket.src, lastPacket.dst, 
            first.seqNumber, lastPacket.seqNumber, function(err) {
              if (err) {
                cb(err);
              } else {
                cb(null, finalPacket);
              }
            });
        }
      });
    } else {
      cb();
    }
  });
}

function handleRemainingPacket(endpoint, packet) {
  ep.withTransaction(endpoint.db, function(T, cb) {
    ep.storePacket(T, packet, function(err) {
      if (err) {
        cb(err);
      } else {
        attemptToAssemblePacket(T, endpoint, packet, cb);
      }
    });
  }, function(err) {
    if (err) {
      console.log('ERROR in handleRemainingPacket:');
      console.log(err);
    }
  });
}

function largePacketHandler(endpoint, packet) {
  if (packet.label == common.firstPacket) {
    handleFirstPacket(endpoint, packet);
  } else if (packet.label == common.remainingPacket) {
    handleRemainingPacket(endpoint, packet);
  }
}

module.exports.splitBuffer = splitBuffer;
module.exports.encodeFirstPacket = encodeFirstPacket;
module.exports.decodeFirstPacket = decodeFirstPacket;
module.exports.encodeRemainingPacket = encodeRemainingPacket;
module.exports.decodeRemainingPacket = decodeRemainingPacket;
module.exports.sendPacket = sendPacket;
module.exports.largePacketHandler = largePacketHandler;

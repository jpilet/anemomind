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

function sendRemainingPacket(dst, seqNumber, localEndpoint, data0, cb) {
  var data = encodeRemainingPacket(seqNumber, data0);
  localEndpoint.sendSimplePacketAndReturn(dst, common.remainingPacket, data, cb);
}

function sendRemainingPackets(dst, seqNumber, localEndpoint, packets, cb) {
  if (packets.length == 0) {
    cb();
  } else {
    sendRemainingPacket(dst, seqNumber, localEndpoint, packets[0], function(err) {
      if (err) {
        cb(err);
      } else {
        sendRemainingPackets(dst, seqNumber, localEndpoint, packets.slice(1), cb);
      }
    });
  }
}

function sendFirstPacket(n, dst, localEndpoint, label, cb) {
  var data = encodeFirstPacket(label, n);
  localEndpoint.sendSimplePacketAndReturn(dst, common.firstPacket, data, function(err, out) {
    if (err) {
      cb(err);
    } else if (out == null) {
      cb(new Error('No map with data returned from sendSimplePacketAndReturn'));
    } else if (out.seqNumber == null) {
      cb(new Error('No sequence number obtained'));
    } else if (typeof out.seqNumber != 'string') {
      cb(new Error('Seq number is not a string'));
    } else {
      cb(null, out.seqNumber);
    }
  });
}

function sendPackets(dst, localEndpoint, label, packets, cb) {
  if (packets.length == 0) {
    cb();
  } else {
    sendFirstPacket(packets.length, dst, localEndpoint, 
                    label, function(err, seqNumber) {
      if (err) {
        cb(err);
      } else if (seqNumber == null) {
        cb(new Error("Missing seqNumber passed from sendFirstPacket"));
      } else {
        sendRemainingPackets(dst, seqNumber, localEndpoint, packets, function(err) {
          if (err) {
            cb(err);
          } else {
            cb(null, seqNumber);
          }
        });
      }
    });
  }
}

function sendPacket(localEndpoint, dst, label, data, settings, cb) {
  if (validSendPacketData(dst, localEndpoint, label, data, settings, cb)) {
    var packets = splitBuffer(data, settings.mtu);
    sendPackets(dst, localEndpoint, label, packets, function(err, seqNumber) {
      if (err) {
        cb(err);
      } else {
        cb(null, { // Try to return the same stuff as would localEndpoint.sendPacket
          src: localEndpoint.name,
          dst: dst,
          label: label,
          seqNumber: seqNumber, // The first seqNumber assigned to the chain of packets.
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

var firstBaseName = 'first.dat';

function handleFirstPacket(partsPath, endpoint, packet) {
  preparePath(partsPath, packet.src, packet.seqNumber, function(err, path) {
    if (err) {
      console.log('ERROR in largepacket.handleFirstPacket: Failed to create ' + path);
    } else {
      var firstName = Path.join(path, firstBaseName);
      fs.writeFile(firstName, packet.data, function(err) {
        if (err) {
          console.log('ERROR in largepacket.handleFirstPacket: Failed to write ' + firstName);
        }
      });
    }
  });
}

function isPartName(s) {
  if (s.length > 4) {
    return s.slice(0, 4) == 'part';
  }
  return false;
}

function listParts(path, cb) {
  fs.readdir(path, function(err, names) {
    if (err) {
      cb(err);
    } else {
      cb(null, names.filter(isPartName).sort());
    }
  });
}

function assemblePacketData(path, partNames, cb) {
  dstFilename = path + "largepacket.dat";
  cmd = 'cat ' + 
    partNames.map(function(name) {return Path.join(path, name);}).join(' ')
    + ' > ' + dstFilename;
  exec(cmd, function(err, stdout, stderr) {
    if (err) {
      cb(err);
    } else {
      fs.readFile(dstFilename, cb);
    }
  });
}

function checkIfComplete(seqNumber, packet0, path, endpoint) {
  var firstName = Path.join(path, firstBaseName);
  fs.readFile(firstName, function(err, data) {
    if (err) {
      console.log('ERROR in largepacket.checkIfComplete: Failed to read ' + firstName);
    } else if (!(data instanceof Buffer)) {
      console.log('ERROR in largepacket.checkIfComplete: The loaded data is not a buffer');
    } else {
      var decoded = decodeFirstPacket(data);
      if (decoded == null) {
        console.log('ERROR in largepacket.checkIfComplete: Failed to decode the first packet');
      } else {
        listParts(path, function(err, partNames) {
          if (partNames.length >= decoded.count) {
            if (partNames.length > decoded.count) {
              console.log('ERROR in largepacket.checkIfComplete: Too many parts received: '
                          + partNames);
            } else {
              // src, dst, seqNumber, label, data
              assemblePacketData(path, partNames, function(err, data) {
                if (err) {
                  console.log('ERROR in largepacket.checkIfComplete: '
                              + 'Failed to assemble packet data from ' + partNames);
                } else {
                  rmdir(path, function(err) {
                    if (err) {
                      console.log('ERROR in largepacket.checkIfComplete: '
                                  + 'Failed to rmdir ' + path + ': ' + err);
                    } else {
                      var packet = {
                        src: packet0.src,
                        dst: packet0.dst,
                        label: decoded.label,
                        seqNumber: seqNumber,
                        data: data
                      };
                      endpoint.callPacketHandlers(packet);
                    }
                  });
                }
              });
            }
          }
        });
      }
    }
  });
}

function handleRemainingPacket(partsPath, endpoint, packet) {
  var decoded = decodeRemainingPacket(packet.data);
  if (decoded == null) {
    console.log('ERROR in largepacket.handleRemainingPacket: Failed to decode packet');
  } else {
    preparePath(partsPath, packet.src, decoded.seqNumber, function(err, path) {
      if (err) {
        console.log('ERROR in largepacket.handleRemainingPacket: Failed to create ' + path);
      } else {
        var filename = Path.join(path, 'part' + packet.seqNumber + '.dat');
        fs.writeFile(filename, decoded.data, function(err) {
          if (err) {
            console.log('ERROR in largepacket.handleRemainingPacket: Failed to write ' + filename);
          } else {
            checkIfComplete(decoded.seqNumber, packet, path, endpoint);
          }
        });
      }
    });
  }
}

function largePacketHandler(endpoint, packet) {
  var partsPath = endpoint.getPartsPath();
  if (packet.label == common.firstPacket) {
    handleFirstPacket(partsPath, endpoint, packet);
  } else if (packet.label == common.remainingPacket) {
    handleRemainingPacket(partsPath, endpoint, packet);
  }
}

module.exports.splitBuffer = splitBuffer;
module.exports.encodeFirstPacket = encodeFirstPacket;
module.exports.decodeFirstPacket = decodeFirstPacket;
module.exports.encodeRemainingPacket = encodeRemainingPacket;
module.exports.decodeRemainingPacket = decodeRemainingPacket;
module.exports.sendPacket = sendPacket;
module.exports.largePacketHandler = largePacketHandler;

var common = require('./common.js');
var Q = require('q');
var fs = require('fs');
var assert = require('assert');
var path = require('path');
var pathIsAbsolute = require('path-is-absolute');
var mkdirp = require('mkdirp');
var mail2 = require('./mail2.sqlite.js');
var msgpack = require('msgpack-js');


function packFile(file) {
  return Q.nfcall(fs.readFile, file.src).then(function(filedata) {
    return {src: filedata, dst: file.dst};
  });
}

function packFiles(fileArray) {
  assert(fileArray instanceof Array);
  return Q.all(fileArray.map(packFile));
}

function resolveFilename(root, filename) {
  if (pathIsAbsolute(filename)) {
    return filename;
  }
  return path.join(root, filename);
}

function unpackFile(root, packedFile) {
  var filename = resolveFilename(root, packedFile.dst);
  var p = path.parse(filename);
  return Q.nfcall(mkdirp, p.dir, 0755)
    .then(Q.nfcall(fs.writeFile, filename, packedFile.src))
  
  // Doesn't Q have a way of wrapping a value directly?
    .then(Q.promised(function() {return filename;}));
}

function unpackFiles(root, packedFileArray) {
  return Q.all(packedFileArray.map(function(data) {return unpackFile(root, data);}));
}

function sendFiles(ep, dstName, fileArray) {
  assert(mail2.isEndPoint(ep));
  return packFiles(fileArray).then(function(packed) {
    return Q.nfinvoke(
      ep, 'sendPacket',
      dstName, common.files, msgpack.encode(packed));
  });
}

function makeFilesHandler(root, verbose) {
  return function(endPoint, packet) {
    if (packet.label == common.files) {
      var packedFileArray = msgpack.decode(packed.data);
      unpackFiles(packedFileArray).then(function(filenames) {
        if (verbose) {
          console.log('Unpacked these files: ');
          console.log(filenames);
        }
      });
    }
  };
}

module.exports.packFile = packFile;
module.exports.packFiles = packFiles;
module.exports.unpackFile = unpackFile;
module.exports.unpackFiles = unpackFiles;
module.exports.sendFiles = sendFiles;

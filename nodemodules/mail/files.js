var common = require('./common.js');
var Q = require('q');
var fs = require('fs');
var assert = require('assert');
var path = require('path');
var pathIsAbsolute = require('path-is-absolute');
var mkdirp = require('mkdirp');


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
    .then(Q.nfcall(fs.writeFile, filename, packedFile.src));
}

function unpackFiles(root, packedFileArray) {
  return Q.all(packedFileArray.map(function(data) {return unpackFile(root, data);}));
}

module.exports.packFile = packFile;
module.exports.packFiles = packFiles;
module.exports.unpackFile = unpackFile;
module.exports.unpackFiles = unpackFiles;

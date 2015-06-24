var common = require('./common.js');
var Q = require('q');
var fs = require('fs');
var assert = require('assert');

function packFile(file) {
  return Q.promised(function(filedata) {return {src: filedata, dst: file.dst}})
  (Q.nfcall(fs.readFile, file.src));
}

function packFiles(fileArray) {
  assert(fileArray instanceof Array);
  return Q.all(fileArray.map(packFile));
}

module.exports.packFile = packFile;
module.exports.packFiles = packFiles;

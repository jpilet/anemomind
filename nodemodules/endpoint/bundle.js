/*

= HOW TO USE =

== ON THE SENDER SIDE ==
bundle.sendBundle(dst, bundleSetup);

where bundleSetup is a map: {
 main: [path to file that should be run, relative to root. Can be .sh or .js]
 dst: [path where the bundle should be unpacked]
 src: [path to .bundle file]
}

== ON THE RECEIVER SIDE ==
The receiver needs to add a bundle handler among the packet handler
of the endpoint:

endpoint.addPacketHandler(bundle.makeBundleHandler());

*/

var mkdirp = require('mkdirp');
var Q = require('q');
var Path = require('path');
var bundlePath = '~/.tmp/bundles';
var tmpFilename = Path.join(bundlePath, 'last.bundle');
var exec = require('child_process').exec;
var msgpack = require('msgpack-js');
var fs = require('fs');
var common = require('./common.js');
var assert = require('assert');

function writeBundleToTempFile(data) {
  return Q.nfcall(mkdirp, bundlePath)
    .then(function() {
      return Q.nfcall(fs.writeFile, tmpFilename, data);
    })
    .then(function() {
      return tmpFilename;
    });
}


function execToMap(cmd, cb) {
  exec(cmd, function(err, stdout, stderr) {
    if (err) {
      cb(err);
    } else {
      cb(null, {
        stdout: stdout,
        stderr: stderr
      });
    }
  });
}

function unpackBundle(endpoint, src, bundleFilename, reposPath) {
  return Q.nfcall(mkdirp, reposPath)
    .then(function() {
      Q.nfcall(common.exists, Path.join(reposPath, '.git'))
    }).then(function(e) {
      var cmd = 'cd ' + reposPath + 
          (e? '; git pull ' + bundleFilename + ' master'
           : 'cd ' + reposPath + '; git clone -b master ' + bundleFilename 
           + ' .');
      return Q.nfcall(execToMap, cmd);
    });
}

function bundleHandler(endpoint, packet) {
  if (packet.label == common.bundle) {
    var data0 = packet.data;
    var x = decodeBundle(data0);
    if (!(data.data instanceof Buffer)) {
      console.log('ERROR in bundle.js, bundleHandler: bundleData is not a Buffer, it is ' 
                  + typeof data.bundleData);
    } else if (typeof data.dstPath != 'string') {
      console.log('ERROR in bundle.js, bundleHandler: The destination path is not a string, it is '
                  + data.dstPath);
    } else {
      console.log('SUCCESSFULLY DECODED THE BUNDLE!!!');
      // return writeBundleToTempFile(data)
      //   .then(function(filename) {
      //     assert(typeof filename == 'string');
      //     return unpackBundle(
      //       endpoint, packet.src, 
      //       data.bundleData, data.dstPath, "/tmp/bundlelog.txt");
      //   }).then()
    }
  }
}

function validateBundleSpec(x) {
  if (typeof x != 'object') {
    return new Error('bundleSpec is not an object');
  } else if (typeof x.bundleFilename != 'string') {
    return new Error('bundleFilename is not a string');
  } else if (typeof x.dstPath != 'string') {
    return new Error('dstPath is not a string');
  }
  return null;
}

function encodeBundle(dstPath, data) {
  assert(data instanceof Buffer);
  return msgpack.encode({dstPath: dstPath, data: data});
}

function decodeBundle(b) {
  assert(b instanceof Buffer);
  return msgpack.decode(b);
}

function sendBundle(endpoint, dst, bundleSpec, cb) {
  var err = validateBundleSpec(bundleSpec);
  if (err) {
    cb(err);
  } else {
    return Q.nfcall(fs.readFile, bundleSpec.bundleFilename)
      .then(function(data) {
        return Q.ninvoke(
          endpoint, "sendPacket",
          dst, common.bundle, encodeBundle(bundleSpec.dstPath, data));
      }).nodeify(cb);
  }
}


module.exports.bundleHandler = bundleHandler;
module.exports.sendBundle = sendBundle;
module.exports.encodeBundle = encodeBundle;

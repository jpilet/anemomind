var q = require('q');
var exec = require('child_process').exec;
var path = require('path');
var mkdirp = require('mkdirp');

// Ensure NODE_ENV is defined.
process.env.NODE_ENV = process.env.NODE_ENV || 'development';
var env = require('../../config/environment');


// Common settings
var serverAddress = 'anemomind@vtiger.anemomind.com';
var repositoryPath = '/home/anemobox/anemobox.git'

function ensureBundleLocation(filename) {
  var p = path.dirname(filename);
  return q.nfcall(mkdirp, p);
}


function exec2(cmd, verbose, cb) {
  if (verbose) {
    console.log('exec: ' + cmd);
  }
  exec(cmd, function(err, stdout, stderr) {
    if (err) {
      cb(err);
    } else {
      cb(null, {stdout: stdout, stderr: stderr});
    }
  });
}

// On the form {from: [tag]} or {from: [tag], to: [tag]}
function makeVersionIntervalString(version) {
  if (version) {
    if (typeof version.from == 'string') {
      if (typeof version.to == 'string') {
        return version.from + '..' + version.to;
      }
      return version.from + '..master';
    }
  }
  return null;
}

// A string that describes what should be included in the bundle
function makeVersionString(version) {
  if (typeof version == 'string') {
    return version;
  } else if (typeof version == 'object') {
    var s = makeVersionIntervalString(version);
    if (s != null) {
      return s;
    }
  }
  return 'master';
}

function makeSshCommand(cmd) {
  return 'ssh ' + serverAddress + ' "' + cmd + '"';
}

// Make a bundle named as 'dstFilename'. The 'version' parameter
// can be either a raw string, or a map with the 'from' key, and optionally
// the 'to' key.
function makeBundle(dstFilename, version, verbose) {
  var v = makeVersionString(version);
  var cmd = 'cd ' + repositoryPath + '; '
      + 'git bundle create - ' + v;
  var fullCmd = makeSshCommand(cmd) + ' > ' + dstFilename;
  return ensureBundleLocation(dstFilename)
    .then(function() {
      return q.nfcall(exec2, fullCmd, verbose);
    });
}

// Get all the possible version tags
function getTags(verbose) {
  var cmd = 'cd ' + repositoryPath + '; git tag';
  var fullCmd = makeSshCommand(cmd);
  return q.nfcall(exec2, fullCmd, verbose)
    .then(function(x) {
      return x.stdout.split(/\s+/).filter(function(s) {
        return s.length > 0;
      });
    });
}

module.exports.makeBundle = makeBundle;
module.exports.getTags = getTags;

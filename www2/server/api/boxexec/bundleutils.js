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

// This is a template of the parameters 
// for a bunde
var settings = {
  // You may want to give a unique name, in
  // case 'makeBundle' is called concurrently on the server
  // for different boats.
  //
  // This can be either an absolute path, or a path relative to 
  // env.bundleDir. In either case, the path will be created if it does not exist.
  bundleName: 'tmp.bundle',
  
  // For debugging, verbosity can be turned on.
  verbose: true,

  // Here you can either pass a string, such as 'v3..master' or just 'master',
  // or you can pass an argument map, such as {from: 'v3'} or {from: 'v3', to: 'master'}
  version: 'master'
};

function ensureBundleLocation(filename) {
  var p = path.dirname(filename);
  return q.nfcall(mkdirp, p);
}

function getFullBundleFilename(bundleName) {
  if (path.isAbsolute(bundleName)) {
    return bundleName;
  } else {
    return path.join(env.bundleDir, bundleName);
  }
}

// TODO: Add settings map for 'anemolab'

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



// Make a bundle that is saved to the file, with name
// as in settings.bundleName. The location of the file
// depends on env.bundleDir.
// This function returns the final filename as a promise when done.
function makeBundle(settings) {
  var v = makeVersionString(settings.version);
  var cmd = 'cd ' + repositoryPath + '; '
      + 'git bundle create - ' + v;
  var bname = getFullBundleFilename(settings.bundleName);
  var fullCmd = makeSshCommand(cmd) + ' > ' + bname;
  return ensureBundleLocation(bname)
    .then(function() {
      return q.nfcall(exec2, fullCmd, settings.verbose);
    })
    .then(function() {
      return bname;
    });
}

// Get all the possible version tags
function getTags(settings) {
  var cmd = 'cd ' + repositoryPath + '; git tag';
  var fullCmd = makeSshCommand(cmd);
  return q.nfcall(exec2, fullCmd, settings.verbose)
    .then(function(x) {
      return x.stdout.split(/\s+/).filter(function(s) {
        return s.length > 0;
      });
    });
}


// These two functions take a settings map as single argument
module.exports.makeBundle = makeBundle;
module.exports.getTags = getTags;

// These are default settings that you can use as a template for
// the arguments passed to the above functions
module.exports.settings = settings;

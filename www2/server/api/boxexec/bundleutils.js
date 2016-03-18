var q = require('q');
var exec = require('child_process').exec;
var path = require('path');

// Common settings
var serverAddress = 'vtiger.anemomind.com';
var repositoryPath = '/home/anemobox/anemobox.git'

// Specific settings
var jonasSettings = {
  localBundleFilename: '/tmp/tmp.bundle',
  verbose: true,
  version: 'master' // {from: 'v0'} // {from: 'v0', to: 'v1'}
};

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

function getUsername(settings, cb) {
  if (settings.user) {
    cb(null, settings.user);
  } else {
    exec2('whoami', settings.verbose, function(err, x) {
      cb(null, x.stdout.trim());
    });
  }
}

function makeRemoteAddress(username) {
  return username + '@' + serverAddress;
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

function makeSshCommand(username, cmd) {
  return 'ssh ' + makeRemoteAddress(username) + ' "' + cmd + '"';
}



// Make a bundle that is saved to the file specified in
// settings.localBundleFilename
function makeBundle(settings) {
  return q.nfcall(getUsername, settings)
    .then(function(username) {
      var v = makeVersionString(settings.version);
      var cmd = 'cd ' + repositoryPath + '; '
          + 'git bundle create - ' 
          + v + ' > /dev/stdout';
      var fullCmd = makeSshCommand(username, cmd) + ' > '
          + settings.localBundleFilename;
      return q.nfcall(exec2, fullCmd, settings.verbose);
    });
}

// Get all the possible version tags
function getTags(settings) {
  return q.nfcall(getUsername, settings)
    .then(function(username) {
      var cmd = 'cd ' + repositoryPath + '; git tag';
      var fullCmd = makeSshCommand(username, cmd);
      return q.nfcall(exec2, fullCmd, settings.verbose);
    })
    .then(function(x) {
      return x.stdout.split(/\s+/).filter(function(s) {
        return s.length > 0;
      });
    });
}

module.exports.makeBundle = makeBundle;
module.exports.getTags = getTags;

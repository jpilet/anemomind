var bundle = require('../bundle.js');
var assert = require('assert');
var Q = require('q');
var mkdirp = require('mkdirp');
var exec = require('child_process').exec;
var Path = require('path');
var fs = require('fs');


function makeDirName(name) {
  return Path.join('/tmp/bundletest', name)
}

function makeDirectory(name) {
  return Q.nfcall(mkdirp, makeDirName(name));
}

function execInDir(name, cmd) {
  return Q.nfcall(exec, 'cd ' + makeDirName(name) + '; ' + cmd);
}

function gitInit(name) {
  return execInDir(name, 'git init');
}

function checkIsRepository(name) {
  return Q.nfcall(fs.access, Path.join(makeDirName(name), '.gitk'), fs.F_OK);
}

function makeRepository(name) {
  return makeDirectory(name)
    .then(gitInit(name))
    .then(checkIsRepository(name));
}

function prepareTestSetup() {
  return makeRepository('rulle');
}

describe('bundle', function() {
  it('Should prepare a setup where we can experiment', function(done) {
    prepareTestSetup()
      .then(function(value) {
        done();
      })
      .catch(function(err) {
        done(err);
      }).done();
  });
});

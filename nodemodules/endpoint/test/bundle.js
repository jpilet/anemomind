var bundle = require('../bundle.js');
var assert = require('assert');
var Q = require('q');
var mkdirp = require('mkdirp');
var exec = require('child_process').exec;
var Path = require('path');


function makeDirName(name) {
  return Path.join('/tmp/bundletest', name)
}

function makeDirectory(name) {
  return Q.nfcall(mkdirp, makeDirName(name));
}

function gitInit(name) {
  var x = makeDirName(name);
  return Q.nfcall(exec, 'cd ' + x + '; git init');
}

function makeRepository(name) {
  return makeDirectory(name)
    .then(gitInit(name));

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

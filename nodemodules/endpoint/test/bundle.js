var bundle = require('../bundle.js');
var assert = require('assert');
var Q = require('q');
var mkdirp = require('mkdirp');
var exec = require('child_process').exec;
var Path = require('path');
var fs = require('fs');

function fn(expr) {
  return function() {return eval(expr);}
}

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

function exists(p, cb) {
  fs.access(p, fs.F_OK, function(err) {
    if (err) {
      console.log("Doesn't exist");
      cb(err);
    } else {
      console.log("Exists");
      cb();
    }
  });
}

function disp(label, promise) {
  var x = Q.defer();
  promise.then(function(value) {
    console.log(label + ' resolves to ');
    console.log(value);
    x.resolve(value);
  }).catch(function(err) {
    console.log(label + ' produces error');
    console.log(err);
    x.reject(err);
  });
  return x.promise;
}

function checkIsRepository(name) {
  var dst = Path.join(makeDirName(name), '.git');
  return Q.nfcall(exists, dst);
}

function makeRepository(name) {
  return makeDirectory(name)
    .then(function() {return gitInit(name);})
    .then(function() {return checkIsRepository(name);});
}

function commitChanges(name, msg) {
  return execInDir(name, 'git add *; git commit -a -m "' + msg + '"');
}

function prepareTestSetup() {
  var name = 'abc';
  return makeRepository(name)
    .then(function() {
      return Q.nfcall(
        fs.writeFile,
        Path.join(makeDirName(name), "main.cpp"), 
        '#include <iostream>\nint main() {\n  std::cout << "Anemomind!" << std::endl;\n}');
    })
    .then(function() {return commitChanges(name, "Initial commit");});
}

describe('bundle', function() {
  it('Should prepare a setup where we can experiment', function(done) {
    prepareTestSetup()      
      .then(function(value) {
        console.log('It is OK: ' + value);
        done();
      }).catch(function(err) {
        console.log('Failed: ' + err);
        done(err);
      }).done();
  });
});

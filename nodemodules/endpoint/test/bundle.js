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
  var base = '/tmp/bundletest';
  if (name) {
    return Path.join(base, name)
  }
  return base;
}


function cleanAll(cb) {
  exec('rm ' + makeDirName() + ' -rf', function(err) {
    cb(); // Ignore whatever error there may be.
  });
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

function createBundle(srcName, outputName, fromTag, nowTag) {
  var createTagCmd = 'git tag -f ' + nowTag + ' master';
  return execInDir(
    srcName, 'git bundle create ' 
      + outputName + ' ' 
      + (fromTag == null? "master; " : fromTag + "..master; ")
      + createTagCmd);
}

function prepareTestSetup() {
  var name = 'src';

  // Make the initial version of the repository
  return Q.nfcall(cleanAll)
    .then(function() {return makeRepository(name);})
    .then(function() {
      return Q.nfcall(
        fs.writeFile,
        Path.join(makeDirName(name), "main.cpp"), 
        '#include <iostream>\nint main() {\n  std::cout << "Anemomind!" << std::endl;\n}');
    })
    .then(function() {return commitChanges(name, "Initial commit");})

  // Create the first bundle
    .then(function() {
      return createBundle(name, 'first.bundle', null, 'v1');
    })

  // Update our source repository
    .then(function() {
      return Q.nfcall(
        fs.writeFile,
        Path.join(makeDirName(name), "main.cpp"), 
        '#include <iostream>\nint main() {\n  '
          +'std::cout << "Anemomind!" << std::endl;\n  return 0;\n}');
    })
    .then(function() {
      return commitChanges(
        name, "Add return statement to main function");
    })

  // Create the second bundle with the updates
    .then(function() {
      return createBundle(name, 'second.bundle', 'v1', 'v2');
    });
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

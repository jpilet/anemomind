var bundle = require('../bundle.js');
var endpoint = require('../endpoint.sqlite.js');
var assert = require('assert');
var Q = require('q');
var mkdirp = require('mkdirp');
var exec = require('child_process').exec;
var Path = require('path');
var fs = require('fs');
var common = require('../common.js');

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

function existsInDir(name, p, cb) {
  return common.exists(Path.join(makeDirName(name), p), cb);
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
  return Q.nfcall(common.exists, dst)
    .then(function(e) {
      assert(e);
    });
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


function makeEndpoint(name) {
  return Q.nfcall(endpoint.tryMakeAndResetEndpoint, 
                  '/tmp/bundletest/' + name + '.db', name);
}

function makeReceiver(name) {
  return makeEndpoint(name)
    .then(function(ep) {
      ep.addPacketHandler(bundle.bundleHandler);
      return ep;
    });
}

function makeEndpoints() {
  return Q.all([
    makeEndpoint('src'),
    makeReceiver('dst0'),
    makeReceiver('dst1')
  ]);
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
    })

  // Check for existence of the bundles
    .then(function() {
      return Q.all([
        Q.nfcall(existsInDir, name, 'first.bundle'),
        Q.nfcall(existsInDir, name, 'second.bundle')
      ]);
    })
    .then(function(values) {
      assert(values.length == 2);
      assert(values[0]);
      assert(values[1]);
    })
    .then(function() {
      return makeEndpoints();
    });
}

function basicSendAndReceive(endpoints) {
  var src = endpoints[0];
  var dst0 = endpoints[1];
  var dst1 = endpoints[2];
  return Q.ninvoke(src, 'getTotalPacketCount')
    .then(function(n) {
      assert(n == 0);
    })
    .then(function() {
      return Q.nfcall(bundle.sendBundle, src, dst0.name, {
        dstPath: makeDirName('dst0'),
        bundleFilename: Path.join(makeDirName('src'), 'first.bundle')
      })
    });
}

describe('bundle', function() {
  it('Should prepare a setup where we can experiment', function(done) {
    prepareTestSetup()      
      .then(function(value) {
        assert(value instanceof Array);
        assert(value.length == 3);
        return basicSendAndReceive(value);
      }).nodeify(done);
  });
});

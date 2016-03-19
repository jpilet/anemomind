var bundle = require('../bundle.js');
var endpoint = require('../endpoint.sqlite.js');
var assert = require('assert');
var Q = require('q');
var mkdirp = require('mkdirp');
var exec = require('child_process').exec;
var Path = require('path');
var fs = require('fs');
var common = require('../common.js');
var sync2 = require('../sync2.js');
var deploybundle = require('../deploybundle.js');
var script = require('../script.js');

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

function makeReceiver(name, cb) {
  return makeEndpoint(name)
    .then(function(ep) {
      ep.addPacketHandler(script.makeScriptRequestHandler(cb));
      return ep;
    });
}

function makeEndpoints(cb) {
  return Q.all([
    makeEndpoint('src'),
    makeReceiver('dst0', cb),
    makeReceiver('dst1', cb)
  ]);
}


function prepareTestSetup(doneCB) {
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
      return makeEndpoints(doneCB);
    });
}

function wait(time, cb) {
  setTimeout(cb, time);
}

function expectError(fun, args, cb) {
  fun.apply(null, args.concat([function(err, output) {
    if (err) {
      cb();
    } else {
      cb(new Error('We expected an error, but no error was produced.'));
    }
  }]));
}

function basicDeployment(endpoints) {
  var src = endpoints[0];
  var dst0 = endpoints[1];
  var dst1 = endpoints[2];
  var test0 = 'test0';
  var test1 = 'test1';
  var test2 = 'test2';
  var firstPath = Path.join(makeDirName('src'), 'first.bundle');
  var secondPath = Path.join(makeDirName('src'), 'second.bundle');

  return makeDirectory(test0)

  // Lets first perform a deployment that should work
    .then(function() {
      return gitInit(test0);
    })
    .then(function() {
      return Q.nfcall(existsInDir, test0, 'main.cpp');
    })
    .then(function(e) {
      assert(!e);
    })
    .then(function() {
      return Q.nfcall(deploybundle.deploy, 
                      firstPath,
                      makeDirName(test0));
    })
    .then(function(value) {
      assert(typeof value == 'object');
      assert(typeof value.currentVersion == 'string');
    }).then(function(value) {
      return Q.all([
        Q.nfcall(existsInDir, test0, 'main.cpp'),
        Q.nfcall(existsInDir, test0 + '_updated', 'main.cpp'),
        Q.nfcall(common.exists, makeDirName(test0) + '_backup')
      ]);
    })
    .then(function(arr) {
      assert(arr[0]);
      assert(!arr[1]);
      assert(!arr[2]);
    })

  // Now lets perform a deployment that should break
  // because the local folder doesn't exist
    .then(function() {
      return Q.nfcall(expectError, deploybundle.deploy, [firstPath, test1]);
    }).then(function(e) {
      // Great, an error was produced as expected. And check that we didnt make any garbage
      return Q.all([
        Q.nfcall(common.exists, makeDirName(test1)),
        Q.nfcall(common.exists, makeDirName(test1) + '_updated'),
        Q.nfcall(common.exists, makeDirName(test1) + '_backup'),
      ]);
    }).then(function(e) {
      for (var i in e) {
        assert(!e[i]);
      }
    })

  // Now lets perform a deployment that should break
  // because the local version is incompatible
    .then(function() {
      return makeDirectory(test2);
    })
    .then(function() {
      return gitInit(test2);
    })
    .then(function() {
      return Q.nfcall(common.exists, makeDirName(test2));
    })
    .then(function(e) {
      assert(e);
    })
    .then(function() {
      return Q.nfcall(fs.writeFile, Path.join(makeDirName(test2), 'Makefile'),
                      'g++ main.cpp');
                      
    })
    .then(function() {
      return commitChanges(test2, 'Add a Makefile');
    })
    .then(function(e) {
      return Q.nfcall(expectError, deploybundle.deploy, [secondPath, test2]);
    }).then(function(e) {
      return Q.all([
        Q.nfcall(common.exists, makeDirName(test2)),
        Q.nfcall(common.exists, makeDirName(test2) + '_updated'),
        Q.nfcall(common.exists, makeDirName(test2) + '_backup'),
        Q.nfcall(existsInDir, test2, 'Makefile'),
        Q.nfcall(existsInDir, test2, 'main.cpp')
      ]);      
    })
    .then(function(e) {
      assert(e[0]);
      assert(!e[1]);
      assert(!e[2]);
      assert(e[3]);
      assert(!e[4]);
    });
}

describe('bundle', function() {
  it('Verify the deploy function', function(done) {
    prepareTestSetup()      
      .then(function(value) {
        assert(value instanceof Array);
        assert(value.length == 3);
        return basicDeployment(value);
      }).nodeify(done);
  });

  it('Compile bundlescript', function(done) {
    bundle.compileBundleScript('/tmp/a.bundle', '/tmp/b')
      .then(function(value) {
        assert(typeof value == 'string');

        var expectedLastPart = 
            'module.exports.main = makeMainFunction("/tmp/a.bundle", "/tmp/b");';
        var n = expectedLastPart.length;
        assert(n < value.length);
        assert.equal(value.slice(value.length - n), expectedLastPart);
      }).nodeify(done);
  });

  it('Deploy a remote bundle', function(done) {
    var d = 'anemobox';
    var full = makeDirName(d);
    var first = Path.join(makeDirName('src'), 'first.bundle');
    prepareTestSetup(receivedScript)
      .then(function(endpoints) {
        var src = endpoints[0];
        var dst = endpoints[1];

        makeRepository(d)
          .then(function() { // Check that we start clean.
            return Q.nfcall(existsInDir, d, 'main.cpp');
          })
          .then(function(e) {
            assert(!e);

            // Initiate the request
            return Q.nfcall(
              bundle.deployRemoteBundle,
              src, dst.name, 
              'testbundle119', // <-- This is a tag we give the script to keep track of it
              first, full);

          })
          .then(function() {
            return Q.nfcall(sync2.synchronize, src, dst);
          });
      }).catch(function(e) {
        if (e) {
          done(e);
        }
      });

    // Once the script is received on the remote endpoint
    function receivedScript() {

      // Check that the bundle was successfully deployed
      return Q.nfcall(existsInDir, d, 'main.cpp')
        .then(function(e) {
          assert(e);
        }).nodeify(done);
    }
  });
});



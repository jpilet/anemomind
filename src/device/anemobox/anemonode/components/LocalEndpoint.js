var mb = require('endpoint/endpoint.sqlite.js');
var naming = require('endpoint/naming.js');
var file = require('endpoint/logfile.js');
var schema = require('endpoint/endpoint-schema.js');
var mkdirp = require('mkdirp');
var boxId = require('./boxId.js');
var config = require('./config.js');
var fs = require('fs');
var path = require('path');
var assert = require('assert');
var DelayedCall = require('./DelayedCall.js');
var sync = require('./fssync.js').sync;

// The path '/media/sdcard/' is also used in logger.js
var mailRoot = '/media/sdcard/mail2/';
var doRemoveLogFiles = false;
var sentName = 'sentlogs';
var closeTimeoutMillis = 30000;
var script = require('endpoint/script.js');
var triggerSync = require('./sync.js').triggerSync;
var endpoints = {};
var files = require('endpoint/files.js');

//var estimator = require('./estimator.js');

function endpointCount() {
  var counter = 0;
  for (var k in endpoints) {
    counter++;
  }
  return counter;
}


// Get the name of the local endpoint. cb is called with that as the single argument.
function getName(cb) {
  boxId.getAnemoId(function(rawId) {
    var id = rawId.trim();
    cb(naming.makeEndpointNameFromBoxId(id));
  });
}

function replaceSpecialChars(endpointName) {
  return endpointName.replace(/\W/g, function (m) {
    return "_";
  });
}

function makeFilenameFromEndpointName(endpointName) {
  return mailRoot + replaceSpecialChars(endpointName)
    + ".sqlite.db";
}

function registerEndpoint(endpointName, endpoint) {
  endpointData = {
    endpoint:endpoint,
    close: new DelayedCall(function() {endpoint.close(function(err) {
      if (err) {
        console.log('Delayed call to close endpoint with name ' + endpointName + ' failed.');
        console.log(err);
      }
    })})
  };
  endpoints[endpointName] = endpointData;
  if (endpointCount() > 1) {
    console.log('WARNING: More than one end point endpoint opened.');
    console.log('Opened endpoints:');
    for (var k in endpoints) {
      console.log('  ' + k);
    }
  }
  return endpointData;
}

function handleIncomingFiles(files) {
  console.log('Received files: ' + files.join(', '));

  console.log("UNCOMMENT THIS in LocalEndpoint.js");
  /*for (var i in files) {
    if (path.resolve(files[i]) == path.resolve(estimator.calibFilePath())) {
      console.log('Loading calibration from ' + files[i]);
      estimator.loadCalib(files[i]);
    }
  }*/

  // flush caches to disk
  sync();
}

function openNewEndpoint(endpointName, cb) {
  mkdirp(mailRoot, 0755, function(err) {
    if (err) {
      cb(err);
    } else {
      var filename = makeFilenameFromEndpointName(endpointName);
      mb.tryMakeEndpoint(
	filename,
	endpointName, function(err, endpoint) {
          if (err) {
            cb(err);
          } else {
            //schema.makeVerbose(endpoint);
            endpoint.addPacketHandler(
              script.makeScriptRequestHandler(triggerSync));
            endpoint.addPacketHandler(
              files.makePacketHandler(config.getConfigPath(), handleIncomingFiles));
            var data = registerEndpoint(endpointName, endpoint);
            data.close.callDelayed(closeTimeoutMillis);
            cb(null, endpoint);
          }
        });
    }
  });
}

// Open a endpoint with a particular name. Usually, this should
// be the one obtained from 'getName'.
function openWithName(endpointName, cb) {
  endpointName = endpointName.trim();
  var data = endpoints[endpointName];
  if (data) {
    assert(data.endpoint);
    data.endpoint.open(function(err, db) {
      data.close.callDelayed(closeTimeoutMillis);
      cb(null, data.endpoint);
    });
  } else {
    openNewEndpoint(endpointName, cb);
  }
}

// Open a local endpoint. cb is called with (err, endpoint)
function open(cb) {
  getName(function(endpointName) {
    openWithName(endpointName, cb);
  });
}

function getBoatId(cb) {
  config.get(function(err, cfg) {
    if (err) {
      cb(err);
    } else {
      if (cfg && cfg.boatId) {
        cb(null, cfg.boatId);
      } else {
        cb(new Error('Missing boat id'));
      }
    }
  });
}

function getServerSideEndpointName(cb) {
  getBoatId(function(err, boatId) {
    if (err) {
      cb(err);
    } else {
      cb(null, naming.makeEndpointNameFromBoatId(boatId));
    }
  });
}

function makeSentLogPathData(logFilePath) {
  var parsed = path.parse(logFilePath);
  var sentDir = path.join(parsed.dir, sentName);
  return {
    srcFilePath: logFilePath, // The file that should be read
    sentDir: sentDir,         // The directory where it should go
    dstFilePath: path.join(sentDir, parsed.base) // Its new filename.
  };
}

function moveLogFileToSent(logfile, cb) {
  var pdata = makeSentLogPathData(logfile);
  mkdirp(pdata.sentDir, function(err) {
    if (err) {
      cb(err);
    } else {
      fs.rename(pdata.srcFilePath, pdata.dstFilePath, cb);
    }
  });
}

function postLogFilesSub(endpoint, dst, paths, cb) {
  assert(typeof cb == 'function');
  if (paths.length == 0) {
    // flush posted files to disk
    sync();
    cb();
  } else {
    var logFilename = paths[0];
    file.sendLogFile(
      endpoint, dst, logFilename,
      function(err) {
        if (err) {
          cb(err);
        } else {
          moveLogFileToSent(logFilename, function(err) {
            if (err) {
              cb(err);
            } else {
              postLogFilesSub(endpoint, dst, paths.slice(1), cb);
            }
          });
        }
      });
  }
}

function postLogFilesForEndpoint(endpoint, paths, cb) {
  getServerSideEndpointName(function(err, dst) {
    if (err) {
      cb(err);
    } else {
      postLogFilesSub(endpoint, dst, paths, function(err) {        
        cb(err, paths);
      });
    }
  });  
}

function postLogFiles(paths, cb) {
  withLocalEndpoint(function(endpoint, done) {
    postLogFilesForEndpoint(endpoint, paths, done);
  }, cb);
}

function postLogFile(path, cb) {
  postLogFiles([path], cb);
}

function isLogFilename(x) {
  // In the logs directory, we expect only log files
  // and a directory named by the variable sentName.
  return x != sentName;
}

function listLogFilesNotPostedForEndpoint(endpoint, logRoot, cb) {
  fs.readdir(logRoot, function(err, logFilesInDir) {
    if (err) {
      cb(err);
    } else {
      cb(null, logFilesInDir.filter(isLogFilename).map(function(fname) {
        return path.join(logRoot, fname);
      }));
    }
  });
}

function withLocalEndpointSub(openFun, cbOperationOnEndpoint, cbResults) {
  
  assert(typeof openFun == 'function');
  assert(typeof cbOperationOnEndpoint == 'function');
  assert(typeof cbResults == 'function');
  
  openFun(function(err, endpoint) {
    if (err) {
      cbResults(err);
    } else {
      cbOperationOnEndpoint(endpoint, cbResults);
    }
  });
}

function withLocalEndpoint(cbOperationOnEndpoint, cbResults) {
  withLocalEndpointSub(open, cbOperationOnEndpoint, cbResults);
}

function withNamedLocalEndpoint(name, cbOperationOnEndpoint, cbResults) {
  withLocalEndpointSub(function(cb) {
    openWithName(name, cb);
  }, cbOperationOnEndpoint, cbResults);
}

function withNamedLocalEndpointValidated(name, cbOperationOnEndpoint, cbResults) {
  getName(function(localName) {
    if (localName.trim() == name.trim()) {
      withNamedLocalEndpoint(name, cbOperationOnEndpoint, cbResults);
    } else {
      cbResults(
        new Error('The local endpoint is named ' + localName + ' but you'
                  + ' are attempting to access an endpoint named ' + name));
    }
  });
}

function listLogFilesNotPosted(logRoot, cb) {
  withLocalEndpoint(function(endpoint, done) {
    listLogFilesNotPostedForEndpoint(endpoint, logRoot, done);
  }, cb);
}

function postRemainingLogFiles(logRoot, cb) {
  withLocalEndpoint(function(endpoint, done) {
    listLogFilesNotPostedForEndpoint(endpoint, logRoot, function(err, files) {
      if (err) {
        done(err);
      } else {
        postLogFilesForEndpoint(endpoint, files, done);
      }
    });
  }, cb);
}

function setRemoveLogFiles(p) {
  doRemoveLogFiles = p;
}

function reset(cb) {
  withLocalEndpoint(function(endpoint, done) {
    endpoint.reset(done);
  }, cb);
}


// Convenient when doing unit tests and we don't have an SD card.
module.exports.setMailRoot = function(newMailRoot) {
  mailRoot = newMailRoot;
}



module.exports.reset = reset;
module.exports.getName = getName;
module.exports.postLogFile = postLogFile;
module.exports.setRemoveLogFiles = setRemoveLogFiles;
module.exports.getServerSideEndpointName = getServerSideEndpointName;
module.exports.listLogFilesNotPosted = listLogFilesNotPosted;
module.exports.withLocalEndpoint = withLocalEndpoint;
module.exports.withNamedLocalEndpoint = withNamedLocalEndpoint;
module.exports.withNamedLocalEndpointValidated
  = withNamedLocalEndpointValidated;
module.exports.postRemainingLogFiles = postRemainingLogFiles;

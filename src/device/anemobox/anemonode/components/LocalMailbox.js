var mb = require('mail/mail2.sqlite.js');
var naming = require('mail/naming.js');
var file = require('mail/file.js');
var schema = require('mail/endpoint-schema.js');
var mkdirp = require('mkdirp');
var boxId = require('./boxId.js');
var config = require('./config.js');
var fs = require('fs');
var path = require('path');
var assert = require('assert');
var DelayedCall = require('./DelayedCall.js');

// The path '/media/sdcard/' is also used in logger.js
var mailRoot = '/media/sdcard/mail2/';
var doRemoveLogFiles = false;
var sentName = 'sentlogs';
var closeTimeoutMillis = 30000;
var script = require('mail/script.js');
var triggerSync = require('./sync.js').triggerSync;
var mailboxes = {};

function mailboxCount() {
  var counter = 0;
  for (var k in mailboxes) {
    counter++;
  }
  return counter;
}


// Get the name of the local mailbox. cb is called with that as the single argument.
function getName(cb) {
  boxId.getAnemoId(function(rawId) {
    var id = rawId.trim();
    cb(naming.makeMailboxNameFromBoxId(id));
  });
}

function replaceSpecialChars(mailboxName) {
  return mailboxName.replace(/\W/g, function (m) {
    return "_";
  });
}

function makeFilenameFromMailboxName(mailboxName) {
  return mailRoot + replaceSpecialChars(mailboxName)
    + ".sqlite.db";
}

function registerMailbox(mailboxName, mailbox) {
  mailboxData = {
    mailbox:mailbox,
    close: new DelayedCall(function() {mailbox.close(function(err) {
      if (err) {
        console.log('Delayed call to close mailbox with name ' + mailboxName + ' failed.');
        console.log(err);
      }
    })})
  };
  mailboxes[mailboxName] = mailboxData;
  if (mailboxCount() > 1) {
    console.log('WARNING: More than one end point mailbox opened.');
    console.log('Opened mailboxes:');
    for (var k in mailboxes) {
      console.log('  ' + k);
    }
  }
  return mailboxData;
}


function openNewMailbox(mailboxName, cb) {
  mkdirp(mailRoot, 0755, function(err) {
    if (err) {
      cb(err);
    } else {
      var filename = makeFilenameFromMailboxName(mailboxName);
      mb.tryMakeEndPoint(
	filename,
	mailboxName, function(err, mailbox) {
          if (err) {
            cb(err);
          } else {
            schema.makeVerbose(mailbox);
            mailbox.addPacketHandler(
              script.makeScriptRequestHandler(triggerSync));
            var data = registerMailbox(mailboxName, mailbox);
            data.close.callDelayed(closeTimeoutMillis);
            cb(null, mailbox);
          }
        });
    }
  });
}

// Open a mailbox with a particular name. Usually, this should
// be the one obtained from 'getName'.
function openWithName(mailboxName, cb) {
  mailboxName = mailboxName.trim();
  var data = mailboxes[mailboxName];
  if (data) {
    assert(data.mailbox);
    data.mailbox.open(function(err, db) {
      data.close.callDelayed(closeTimeoutMillis);
      cb(null, data.mailbox);
    });
  } else {
    openNewMailbox(mailboxName, cb);
  }
}

// Open a local mailbox. cb is called with (err, mailbox)
function open(cb) {
  getName(function(mailboxName) {
    openWithName(mailboxName, cb);
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

function getServerSideMailboxName(cb) {
  getBoatId(function(err, boatId) {
    if (err) {
      cb(err);
    } else {
      cb(null, naming.makeMailboxNameFromBoatId(boatId));
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

function postLogFilesSub(mailbox, dst, paths, cb) {
  assert(typeof cb == 'function');
  if (paths.length == 0) {
    cb();
  } else {
    var logFilename = paths[0];
    file.sendLogFile(
      mailbox, dst, logFilename,
      function(err) {
        if (err) {
          cb(err);
        } else {
          moveLogFileToSent(logFilename, function(err) {
            if (err) {
              cb(err);
            } else {
              postLogFilesSub(mailbox, dst, paths.slice(1), cb);
            }
          });
        }
      });
  }
}

function postLogFilesForMailbox(mailbox, paths, cb) {
  getServerSideMailboxName(function(err, dst) {
    if (err) {
      cb(err);
    } else {
      postLogFilesSub(mailbox, dst, paths, function(err) {        
        cb(err, paths);
      });
    }
  });  
}

function postLogFiles(paths, cb) {
  withLocalMailbox(function(mailbox, done) {
    postLogFilesForMailbox(mailbox, paths, done);
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

function listLogFilesNotPostedForMailbox(mailbox, logRoot, cb) {
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

function withLocalMailboxSub(openFun, cbOperationOnMailbox, cbResults) {
  
  assert(typeof openFun == 'function');
  assert(typeof cbOperationOnMailbox == 'function');
  assert(typeof cbResults == 'function');
  
  openFun(function(err, mailbox) {
    if (err) {
      cbResults(err);
    } else {
      cbOperationOnMailbox(mailbox, cbResults);
    }
  });
}

function withLocalMailbox(cbOperationOnMailbox, cbResults) {
  withLocalMailboxSub(open, cbOperationOnMailbox, cbResults);
}

function withNamedLocalMailbox(name, cbOperationOnMailbox, cbResults) {
  withLocalMailboxSub(function(cb) {
    openWithName(name, cb);
  }, cbOperationOnMailbox, cbResults);
}

function listLogFilesNotPosted(logRoot, cb) {
  withLocalMailbox(function(mailbox, done) {
    listLogFilesNotPostedForMailbox(mailbox, logRoot, done);
  }, cb);
}

function postRemainingLogFiles(logRoot, cb) {
  withLocalMailbox(function(mailbox, done) {
    listLogFilesNotPostedForMailbox(mailbox, logRoot, function(err, files) {
      if (err) {
        done(err);
      } else {
        postLogFilesForMailbox(mailbox, files, done);
      }
    });
  }, cb);
}

function setRemoveLogFiles(p) {
  doRemoveLogFiles = p;
}

function reset(cb) {
  withLocalMailbox(function(mailbox, done) {
    mailbox.reset(done);
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
module.exports.getServerSideMailboxName = getServerSideMailboxName;
module.exports.listLogFilesNotPosted = listLogFilesNotPosted;
module.exports.withLocalMailbox = withLocalMailbox;
module.exports.withNamedLocalMailbox = withNamedLocalMailbox;
module.exports.postRemainingLogFiles = postRemainingLogFiles;

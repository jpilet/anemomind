var mb = require('mail/mail.sqlite.js');
var naming = require('mail/naming.js');
var file = require('mail/file.js');
var mkdirp = require('mkdirp');
var boxId = require('./boxId.js');
var config = require('./config.js');
var fs = require('fs');
var path = require('path');
var assert = require('assert');

// The path '/media/sdcard/' is also used in logger.js
var mailRoot = '/media/sdcard/mail/';
var doRemoveLogFiles = false;
var sentName = 'sentlogs';

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


function makeAckHandler() {
  return mb.makePerPacketAckHandler(function(mailbox, packet) {
    if (file.isLogFilePacket(packet)) {
      var msg = file.unpackFileMessage(packet.data);
      if (file.isLogFileInfo(msg.info)) {
        var p = msg.path;
        console.log(
          'This logfile was successfully delivered to the server and can be removed: ' + p);

        // Removing files is a bit scary, so
        // maybe we want to have it disabled to start with.
        // We might lose valuable data. We can enable it once
        // we are very sure everything works.
        if (doRemoveLogFiles) {
          fs.unlink(p, function(err) {
            if (err) {
              console.log('Failed to remove ' + p);
            } else {
              console.log('Removed.');
            }
          });
        } else {
          console.log('It is configured not to be removed.');
        }
      } else {
        console.log('WARNING: Packets labeled logfile should contain logfile data.');
      }
    }
  });
}

function registerMailbox(mailboxName, mailbox) {
  mailboxes[mailboxName] = mailbox;
  if (mailboxCount() > 1) {
    console.log('WARNING: More than one end point mailbox opened.');
    console.log('Opened mailboxes:');
    for (var k in mailboxes) {
      console.log('  ' + k);
    }
  }
}


// Open a mailbox with a particular name. Usually, this should
// be the one obtained from 'getName'.
function openWithName(mailboxName, cb) {
<<<<<<< HEAD
  var alreadyOpened = mailboxes[mailboxName];
  if (alreadyOpened) {
    cb(null, alreadyOpened);
  } else {
    mkdirp(mailRoot, 0755, function(err) {
      if (err) {
        cb(err);
      } else {
        var filename = makeFilenameFromMailboxName(mailboxName);
        mb.tryMakeMailbox(
	  filename,
	  mailboxName, function(err, mailbox) {
            if (err) {
              cb(err);
            } else {
              registerMailbox(mailboxName, mailbox);
              cb(null, mailbox);
            }
          });
      }
    });
  }
=======
  mkdirp(mailRoot, 0755, function(err) {
    if (err) {
      cb(err);
    } else {

      // We could be using a constant mailbox filename
      // if we wanted because there is only one mailbox
      // endpoint on the anemobox, but I believe this is more
      // robust in case we reinstall the anemobox without
      // wiping the contents of the SD card.
      var filename = makeFilenameFromMailboxName(mailboxName);
      mb.tryMakeMailbox(
	filename,
	mailboxName, function(err, mailbox) {
          if (err) {
            cb(err);
          } else {
            mailbox.forwardPackets = false;

            // Maybe it is better to not remove them.
            // After all, they are in the sent folder that we can remove when we like.
            /////mailbox.onAcknowledged = makeAckHandler();
            
            cb(null, mailbox);
          }
        });
    }
  });
>>>>>>> master
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

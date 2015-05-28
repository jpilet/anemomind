var mb = require('mail/mail.sqlite.js');
var naming = require('mail/naming.js');
var file = require('mail/file.js');
var mkdirp = require('mkdirp');
var boxId = require('./boxId.js');
var config = require('./config.js');
var fs = require('fs');

// The path '/media/sdcard/' is also used in logger.js
var mailRoot = '/media/sdcard/mail/';

var doRemoveLogFiles = false;

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
    if (file.isFilePacket(packet)) {
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
      }
    }
  });
}

// Open a mailbox with a particular name. Usually, this should
// be the one obtained from 'getName'.
function openWithName(mailboxName, cb) {
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
            mailbox.onAcknowledged = makeAckHandler();
            cb(null, mailbox);
          }
        });
    }
  });
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

function postLogFile(path, cb) {
  open(function(err, mb) {
    if (err) {
      cb(err);
    } else {
      getServerSideMailboxName(function(err, dst) {
        file.sendFile(
          mb, dst, path,
          file.makeLogFileInfo(), function(err) {
            // Always try to close, even if there was an error.
            mb.close(function(err2) {
              cb(err || err2);
            });
           });
      });
    }
  });
}

function listLogFilesNotPostedForMailbox(mailbox, logRoot, cb) {
  mailbox.getAllPackets(function(err, packets) {
    var unpacked = packets.map(file.unpackIfFilePacket);
  });
}

function setRemoveLogFiles(p) {
  doRemoveLogFiles = p;
}

// Convenient when doing unit tests and we don't have an SD card.
module.exports.setMailRoot = function(newMailRoot) {
  mailRoot = newMailRoot;
}

module.exports.getName = getName;
module.exports.open = open;
module.exports.openWithName = openWithName;
module.exports.postLogFile = postLogFile;
module.exports.setRemoveLogFiles = setRemoveLogFiles;
module.exports.getServerSideMailboxName = getServerSideMailboxName;

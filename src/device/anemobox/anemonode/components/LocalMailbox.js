var mb = require('mail/mail.sqlite.js');
var naming = require('mail/naming.js');
var file = require('mail/file.js');
var mkdirp = require('mkdirp');
var boxId = require('./boxId.js');
var config = require('./config.js');

// The path '/media/sdcard/' is also used in logger.js
var mailRoot = '/media/sdcard/mail/';

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
	mailboxName, cb
      );
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

function postLogFile(path, cb) {
  open(function(err, mb) {
    if (err) {
      cb(err);
    } else {
      getBoatId(function(err, boatId) {
        var dst = naming.makeMailboxNameFromBoatId(boatId);
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

// Convenient when doing unit tests and we don't have an SD card.
module.exports.setMailRoot = function(newMailRoot) {
  mailRoot = newMailRoot;
}

module.exports.getName = getName;
module.exports.open = open;
module.exports.openWithName = openWithName;
module.exports.postLogFile = postLogFile;

var mb = require('mail/mail.sqlite.js');
var naming = require('mail/naming.js');
var mkdirp = require('mkdirp');
var boxId = require('./boxId.js');

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
    mb.tryMakeMailbox(

      // We could be using a constant mailbox filename
      // if we wanted because there is only one mailbox
      // endpoint on the anemobox, but I believe this is more
      // robust in case we reinstall the anemobox without
      // wiping the contents of the SD card.
      makeFilenameFromMailboxName(mailboxName),
      
      mailboxName, cb
    );
  });
}

// Open a local mailbox. cb is called with (err, mailbox)
function open(cb) {
  getName(function(mailboxName) {
    openWithName(mailboxName, cb);
  });
}

// Convenient when doing unit tests and we don't have an SD card.
module.exports.setMailRoot = function(newMailRoot) {
  mailRoot = newMailRoot;
}

module.exports.getName = getName;
module.exports.open = open;

var mb = require('mail/mail.sqlite.js');
var naming = require('mail/naming.js');
var mkdirp = require('mkdirp');
var boxId = require('./boxId.js');

// The path '/media/sdcard/' is also used in logger.js
var mailRoot = '/media/sdcard/mail/';

// Get the name of the local mailbox. cb is called with that as the single argument.
function getName(cb) {
  boxId.getAnemoId(function(id) {
    cb(naming.makeMailboxNameFromBoxId(id));
  });
}

// Open a mailbox with a particular name. Usually, this should
// be the one obtained from 'getName'.
function openWithName(mailboxName, cb) {
  mkdirp(mailRoot, 0755, function(err) {
    var filename = mailRoot + mailboxName;
    tryMakeMailbox(filename, mailboxName, cb);
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

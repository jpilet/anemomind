var mb = require('mail/mail.sqlite.js');
var mkdirp = require('mkdirp');
var boxId = require('components/boxId.js');
var naming = require('mail/naming.js');

// The path '/media/sdcard/' is also used in logger.js
var mailRoot = '/media/sdcard/mail/';

// Get the name of the local mailbox. cb is called with that as the single argument.
function getName(cb) {
  boxId.getAnemoId(function(id) {
    cb(naming.makeMailboxNameFromBoxId(id));
  });
}

// Open a local mailbox. cb is called with (err, mailbox)
function open(cb) {
  getName(function(mailboxName) {
    var filename = mailRoot + localMailboxName;
    tryMakeMailbox(filename, mailboxName, cb);
  });
}

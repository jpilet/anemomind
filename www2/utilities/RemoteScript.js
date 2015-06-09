var mb = require('../server/api/mailrpc/mailbox.js');
var naming = require('mail/naming.js');
var script = require('mail/script.js');

// Used to send a script from a boat to a box.
function executeScriptOnBox(boatId, boxId, type, script, cb) {
  mb.openMailbox(
    naming.makeMailboxNameFromBoatId(boatId),
    function(err, mailbox) {
      script.runRemoteScript(
        mailbox, naming.makeMailboxNameFromBoxId(), type, script, cb);
    });
}

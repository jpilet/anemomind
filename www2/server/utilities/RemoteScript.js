var naming = require('mail/naming.js');
var common = require('mail/common.js');
var appRoot = require('app-root-path');
var Boat = require('../api/boat/boat.model.js');


/*

function getBoxId(boatId, cb) {
  Boat.findById(boatId, function(err, boat) {
    if (err) {
      cb(err);
    } else if (!boat) {
      cb(new Error('No such boat'));
    } else {
      cb(null, boat.anemobox);
    }
  });
}

function listAllBoats(cb) {
  Boat.find({}, cb);
}

// Used to send a script from a boat to a box.
function executeScriptOnBoat(boatId, type, script, cb) {
  mb.openMailbox(
    naming.makeMailboxNameFromBoatId(boatId),
    function(err, mailbox) {
      script.runRemoteScript(
        mailbox, naming.makeMailboxNameFromBoxId(),
        type, script, cb);
    });
}
*/

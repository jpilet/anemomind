// A simulation of mailbox synchronizations
var mb = require("./mail.sqlite.js");
var assert = require('assert');
var async = require("async");
var q = require("q");

var boxnames = ["A", "B", "C"];


function fillWithPackets(count, srcMailbox, dstMailboxName, cb) {
    assert(typeof count == 'number');
    assert(typeof dstMailboxName == 'string');
    if (count == 0) {
	cb();
    } else {
	srcMailbox.sendPacket(
	    dstMailboxName,
	    "Some-label" + count,
	    "some-data " + count,
	    fillWithPackets(count-1, srcMailbox, dstMailboxName, cb)
	);
    }
}

function mailboxesCreated(err, mailboxes) {
    fillWithPackets(
	39,
	mailboxes[0],
	mailboxes[2].mailboxName,
	function() {
	    console.log("Filled with packets");
	}
    );
}



// Main call to run the demo
async.map(
    boxnames,
    function (boxname, cb) {
	var box = new mb.Mailbox(":memory:", boxname, function(err) {
	    cb(err, box);
	});
    },
    mailboxesCreated
);

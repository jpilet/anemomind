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
	    new Buffer(3),
	    function (err) {
		if (err == undefined) {
		    fillWithPackets(count-1, srcMailbox, dstMailboxName, cb)
		} else {
		    cb(err);
		}
	    }
	);
    }
}

// Synchronize state in only one direction,
// so that boxA will know everything that boxB knows,
// but not the other way around.
function synchronizeDirected(boxA, boxB, cb) {
    // First retrieve the first number we should ask for
    box.getForeignStartNumber(
	boxB.mailboxName,
	function(err, startFrom) {
	    if (err == undefined) {
		
		// Retrieve a light-weight packet
		// just to see if we should accept it
		boxB.getFirstPacketStartingFrom(
		    startFrom,
		    function(err, row) {
			if (err == undefined) {

			    if (row == undefined) { // <-- no more data to fetch, we are done.
				cb();
			    } else {

				// Check if this packet is admissible
				
			    }
			} else {
			    cb(err);
			}
		    }
		    true,
		);
	    } else {
		cb(err);
	    }
	}
    );
}

// Perform a full synchronization of the contents in the two mailboxes.
// boxB will share its packets with boxA, and boxA will share its packets with boxB.
function synchronize(boxA, boxB, cb) {
    synchronizeDirected(
	boxA, boxB,
	function(err) {
	    synchronizeDirected(
		boxB, boxA,
		cb
	    );
	}
    );
}

function startSync(err, mailboxes) {

}

// Called once the first mailbox has been filled
function mailboxesCreated(err, mailboxes) {
    fillWithPackets(
	2,
	mailboxes[0],
	mailboxes[2].mailboxName,
	function(err) {
	    startSync(err, mailboxes);
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

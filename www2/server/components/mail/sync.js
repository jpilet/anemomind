// A simulation of mailbox synchronizations
var assert = require('assert');
var bigint = require('./bigint.js');

// See mailbox-schema.js for a list of mailbox methods that
// a mailbox must support.


function finishSync(index, boxA, boxB, cb) {
    // Where are done synchronizing. Save the index and
    // move on.
    boxA.setForeignDiaryNumber(
	boxB.mailboxName,
	index,
	cb
    );
}

function fetchFullPacket(index, boxA, boxB, cb) {
    boxB.getFirstPacketStartingFrom(
	index,
	false,
	function (err, row) {
	    if (err == undefined) {

		if (row == undefined) {
		    console.log('WARNING: We wouldnt expect the packet to be empty');
		    finishSync(index, boxA, boxB, cb);
		} else {

		    // Create a packet object
		    var packet = row;

		    // The actual index of the packet
		    // that we fetched.
		    var newIndex = row.diaryNumber;

		    
		    boxA.handleIncomingPacket(
			packet,
			function (err) {
			    
			    if (err == undefined) {
				
				// Recur with the next index.
				synchronizeDirectedFrom(
				    // IMPORTANT: Call with the index of the
				    // packet fetched here.
				    bigint.inc(newIndex),
				    boxA, boxB, cb
				);
			    } else {
				cb(err);
			    }
			}
		    );
		}
		
	    } else {
		cb(err);
	    }
	}
    );
}

function handleSyncPacketLight(index, lightPacket, boxA, boxB, cb) {
    if (lightPacket == undefined) {
	finishSync(index, boxA, boxB, cb);
    } else { 
	boxA.isAdmissible(
	    lightPacket.src, lightPacket.dst, lightPacket.seqNumber,
	    function(err, admissible) {
		if (err == undefined) {
		    if (admissible) {

			// It is admissible. Let's fetch the full packet.
			fetchFullPacket(
			    index,
			    boxA,
			    boxB,
			    cb
			);
			
		    } else {
			// Recur, with next index.
			synchronizeDirectedFrom(
			    bigint.inc(lightPacket.diaryNumber),
			    boxA, boxB, cb
			);
		    }
		} else {
		    cb(err);
		}
	    }
	);
    }
}

function synchronizeDirectedFrom(startFrom, boxA, boxB, cb) {
    // Retrieve a light-weight packet
    // just to see if we should accept it
    boxB.getFirstPacketStartingFrom(
	startFrom,
	true,
	function(err, row) {
	    if (err == undefined) {
		handleSyncPacketLight(startFrom, row, boxA, boxB, cb);
	    } else {
		cb(err);
	    }	    
	}
    );
}


// Synchronize state in only one direction,
// so that boxA will know everything that boxB knows,
// but not the other way around.
function synchronizeDirected(boxA, boxB, cb) {
    // First retrieve the first number we should ask for
    boxA.getForeignStartNumber(
	boxB.mailboxName,
	function(err, startFrom) {
	    if (err == undefined) {
		synchronizeDirectedFrom(
		    startFrom, boxA, boxB,
		    function() {
			cb();
		    }
		);
	    } else {
		cb(err);
	    }
	}
    );
}


// Perform a full synchronization of the contents in the two mailboxes.
// boxB will share its packets with boxA, and boxA will share its packets with boxB.
//
// These two boxes can be polymorphic: For instance one mailbox can be
// a local one of type mailsqlite, another one a remote one connected with RPC.
function synchronize(boxA, boxB, cb) {
    synchronizeDirected(
	boxA, boxB,
	function(err) {
	    if (err == undefined) {
		synchronizeDirected(
	    	    boxB, boxA,
	    	    cb
		);
	    } else {
		cb(err);
	    }
	}
    );
}

module.exports.synchronize = synchronize;

// A simulation of mailbox synchronizations
var mb = require("../mail.sqlite.js");
var assert = require('assert');
var async = require("async");
var pkt = require("../packet.js");

var boxnames = ["A", "B", "C"];

var VERBOSE = 1;

function disp(x) {
    if (VERBOSE) {
	console.log(x);
    }
}


function fillWithPackets(count, srcMailbox, dstMailboxName, cb) {
    disp('Fill mailbox with name ' + srcMailbox.mailboxName +
		' with ' + count + ' packets intended for ' + dstMailboxName);
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

function getPacketCounts(boxes, cb) {
    async.map(
	boxes,
	function(box, a) {
	    box.getTotalPacketCount(a);
	},
	cb
    );
}

function dispPacketCounts(boxes, cb) {
    getPacketCounts(
	boxes,
	function(err, results) {
	    disp('Packet counts');
	    for (var i = 0; i < results.length; i++) {
		disp('  ' + boxes[i].mailboxName + ': ' + results[i]);
	    }
	    disp('\n\n\n');
	    cb(err);
	}
    );
}




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
		    var packet = new pkt.Packet(
			row.src,
			row.dst,
			row.seqnumber,
			row.cnumber,
			row.label,
			row.data
		    );

		    // The actual index of the packet
		    // that we fetched.
		    var newIndex = row.diarynumber;

		    
		    boxA.handleIncomingPacket(
			packet,
			function (err) {
			    
			    if (err == undefined) {
				
				// Recur with the next index.
				synchronizeDirectedFrom(
				    // IMPORTANT: Call with the index of the
				    // packet fetched here.
				    newIndex + 1,
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
	    lightPacket.src, lightPacket.dst, lightPacket.seqnumber,
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
			    lightPacket.diarynumber + 1,
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

var allMailboxes = undefined;

// Synchronize state in only one direction,
// so that boxA will know everything that boxB knows,
// but not the other way around.
function synchronizeDirected(boxA, boxB, cb) {
    assert(allMailboxes.length == 3);
    // First retrieve the first number we should ask for
    boxA.getForeignStartNumber(
	boxB.mailboxName,
	function(err, startFrom) {
	    if (err == undefined) {
		synchronizeDirectedFrom(
		    startFrom, boxA, boxB,
		    function() {
			disp('Synchronized ' + boxA.mailboxName +
				    ' from ' + boxB.mailboxName + '.]');
			dispPacketCounts(
			    allMailboxes,
			    cb
			);
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

// Perform pairwise synchronization of mailboxes, from left to right.
function synchronizeArray(mailboxes, cb) {
    if (mailboxes.length < 2) {
	cb();
    } else {
	synchronize(
	    mailboxes[0],
	    mailboxes[1],
	    function (err) {
		if (err == undefined) {
		    synchronizeArray(mailboxes.slice(1), cb);
		} else {
		    cb(err);
		}
	    }
	);
    }
}

// function synchronizeForthAndBack(mailboxes, cb) {
//     synchronizeArray(
// 	mailboxes,
// 	function (err) {
// 	    synchronizeArray(
// 		mailboxes.reverse(),
// 		cb
// 	    );
// 	}
//     );
// }

function synchronizeForthAndBack(mailboxes, from, to, cb) {
    if (from < to) {
	var even = from % 2 == 0;
	if (even) {
	    disp('FORWARD SYNCH, from = ' + from);
	} else {
	    disp('BACKWARD SYNCH, from = ' + from);
	}
	var reversed = mailboxes.slice(0).reverse();
	synchronizeArray(
	    (even? mailboxes : reversed),
	    function(err) {
		synchronizeForthAndBack(mailboxes, from+1, to, cb)
	    }
	);
    } else {
	cb();
    }
}


function someSpace(s) {
    for (var i = 0; i < 9; i++) {
	disp(s);
    }
}



function startSync(err, mailboxes, done) {
    if (err == undefined) {

	// This will propage messages from A to C,
	// then propagate messages from C to A.
	synchronizeForthAndBack(
	    mailboxes,
	    0, 2,
	    function (err) {

		getPacketCounts(
		    mailboxes,
		    function(err, counts) {

			// The 9 packets A->C that were not marked as acked,
			// and the 'ack' packet C->A.
			assert(counts[0] == 10);

			// The 39 packets A->C and the 'ack' packet C->A
			assert(counts[1] == 40);
			
			// The 39 packets A->C and the 'ack' packet C->A
			assert(counts[2] == 40);
			
			// Let's send two more packets.
			fillWithPackets(
			    2,
			    mailboxes[0],
			    mailboxes[2].mailboxName,
			    function(err) {
				synchronizeForthAndBack(
				    mailboxes, 0, 2,
				    
				    function(err) {
					done();
				    }			    
				);
			    }
			);
		    }
		);
	    }
	);
    } else {
	console.log('Something failed: %j', err);
    }
}


// Called once the first mailbox has been filled
function mailboxesCreated(mailboxes, done) {

    someSpace('');

    allMailboxes = mailboxes;

    var PACKETCOUNT = 39;
    
    fillWithPackets(
	PACKETCOUNT,
	mailboxes[0],
	mailboxes[2].mailboxName,
	function(err) {
	    assert(err == undefined);
	    startSync(err, mailboxes, done);
	}
    );
}



// Main call to run the demo
describe(
    'Synchronize',
    function() {
	it(
	    'Synchronize three mailboxes with each other',
	    function(done) {

		async.map(
		    boxnames,
		    function (boxname, cb) {
			var box = new mb.Mailbox(":memory:", boxname, function(err) {
			    cb(err, box);
			});
		    },
		    function(err, boxes) {
			assert(err == undefined);
			mailboxesCreated(boxes, done);
		    }
		);

		
	    }
	);
    }
);



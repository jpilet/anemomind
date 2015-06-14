var assert = require('assert');
var pkt = require("./packet.js");
var bigint = require('./bigint.js');
var sync = require('./sync.js');
var async = require("async");

var VERBOSE = 0;
function disp(x) {
    if (VERBOSE) {
	console.log(x);
    }
}


function fillWithPackets(count, srcMailbox, dstMailboxName, cb) {
    disp('Fill mailbox with name ' + srcMailbox.mailboxName +
		' with ' + count + ' packets intended for ' + dstMailboxName);
    assert.equal(typeof count, 'number');
    assert.equal(typeof dstMailboxName, 'string');
    if (count == 0) {
	cb();
    } else {
	srcMailbox.sendPacket(
	    dstMailboxName,
	    49 + count,
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


// Perform pairwise synchronization of mailboxes, from left to right.
function synchronizeArray(mailboxes, cb) {
    if (mailboxes.length < 2) {
	cb();
    } else {
	sync.synchronize(
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
			console.log('counts = ', counts);
			
			// The 9 packets A->C that were not marked as acked,
			// and the 'ack' packet C->A.
			assert.equal(counts[0], 10);

			// The 39 packets A->C and the 'ack' packet C->A
			assert.equal(counts[1], 40);
			
			// The 39 packets A->C and the 'ack' packet C->A
			assert.equal(counts[2], 40);
			
			// Let's send two more packets.
			fillWithPackets(
			    2,
			    mailboxes[0],
			    mailboxes[2].mailboxName,
			    function(err) {
				synchronizeForthAndBack(
				    mailboxes, 0, 2,
				    function(err) {
					getPacketCounts(
					    mailboxes,
					    function(err, counts) {

						// Now the 30 packets that were
						// acked have been removed. What
						// remains are the remaining 9
						// packets A->C that were not acked,
						// the 'ack' packet C->A and the
						// two new packets A->C that were
						// just sent. 12 packets in total.
						assert.equal(counts[0], 12);
						assert.equal(counts[1], 12);
						assert.equal(counts[2], 12);
						
						done();
					    }
					);
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
function synchronizeThreeMailboxes(mailboxes, done) {
  assert.equal(mailboxes.length, 3);
  mailboxes[1].forwardPackets = true;
  someSpace('');
  var PACKETCOUNT = 39;
  fillWithPackets(
    PACKETCOUNT,
    mailboxes[0],
    mailboxes[2].mailboxName,
    function(err) {
      assert.equal(err, undefined);
      startSync(err, mailboxes, done);
    }
  );
}


module.exports.synchronizeThreeMailboxes = synchronizeThreeMailboxes;

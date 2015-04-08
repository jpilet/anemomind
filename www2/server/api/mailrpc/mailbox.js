var mb = require('../../components/mail/mail.sqlite.js');


// Please list below all the callbacks that should be called,
// sequentially, whenever a packet is received
var onPacketReceived = [

    // For instance, if this packet is part of a file being transferred,
    // we might want to do something with this file once all pieces have
    // been received.
    
    // Some example handlers
    function(packet, cb) {
	console.log('First handler: Received packet %j', packet);
	cb();
    },

    function(packet, cb) {
	console.log('Second handler.');
	cb();
    }
    
];

// Please list below all the callbacks that should be called,
// sequentially, whenever an ack packet is received.
var onAcknowledged = [

    // As for onPacketReceived, maybe we want to do something once
    // we receive an acknowledgment packet for packets previously sent.
    //
    // For instance, if we were transferring a file, we might want to delete that file
    // or something once we know all its pieces has reached the destination.
    function(data, cb) {
	console.log('Received acknowledgement for packets previously sent: %j', data);
	cb();
    }

    
];

// Conveniency function for sending a packet:
// It will open a mailbox, send a packet from
// that mailbox, and close the mailbox.
function sendPackets(src, dst, label, dataArray, cb) {
    openMailbox(
	src,
	function(err, mailbox) {
	    if (err) {
		cb(err);
	    } else {
		mailbox.sendPackets(
		    dst,
		    label,
		    dataArray,
		    function (err) {
			if (err) {
			    cb(err);
			} else {
			    mailbox.close(
				cb
			    );			    
			}
		    }
		);
	    }
	}
    );
}

function openMailbox(mailboxName, cb) {
    if (!mb.isValidMailboxName(mailboxName)) {
	cb(new Error('Invalid mailbox name: ' + mailboxName));
    } else {
	var filename = mailboxName + '.mailsqlite.db';
	mb.tryMakeMailbox(
	    filename, mailboxName,
	    function(err, mailbox) {
		if (err) {
		    cb(err);
		} else {
		    mailbox.onPacketReceived = onPacketReceived;
		    mailbox.onAcknowledged = onAcknowledged;
		    cb(err, mailbox);
		}
	    }
	);
    }
}

exports.openMailbox = openMailbox;
exports.sendPacket = sendPacket;

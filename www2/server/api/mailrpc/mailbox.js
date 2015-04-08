var mb = require('../../components/mail/mail.sqlite.js');

function debugPacketHandler(packet, cb) {
    console.log('debugPacketHandler: Received packet %j', packet);
    cb();
}

// Please list below all the callbacks that should be called,
// sequentially, whenever a packet is received
var onPacketReceived = [
    
];

// Please list below all the callbacks that should be called,
// sequentially, whenever an ack packet is received.
var onAcknowledged = [
    
];

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

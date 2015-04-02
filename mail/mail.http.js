// A remote mailbox that we can play with, over HTTP.

var ServerConnection = require('./server-connection.js');


function Mailbox(serverConnection, mailboxName, calls) {
    this.mailboxName = mailboxName;
    for (var i = 0; i < calls.length; i++) {
	var call = calls[i];

	// Add the mailbox name as the first argument.
	// We want talking with a remove mailbox to have
	// the same interface as a local one.
	this[call] = function() {
	    serverConnection[call].apply(
		null,
		[mailboxName].concat(Array.prototype.slice.call(arguments))
	    );
	};
    }
}


// Call this function when you need a new mailbox.
function tryMakeMailbox(serverAddress, userdata, mailboxName, cb) {
    var s0 = new ServerConnection(serverAddress);
    s0.login(userdata, function(err, s) {
	if (err) {
	    cb(err);
	} else {
	    var calls = [
		// Calls required for synchronization
		'setForeignDiaryNumber',
		'getFirstPacketStartingFrom',
		'handleIncomingPacket',
		'isAdmissible',
		'getForeignDiaryNumber',
		'getMailboxName',

		// Calls for other functions
		'sendPacket',
		'getTotalPacketCount'
	    ];

	    // Register these as rpc calls.
	    s.registerCalls(calls);
	    
	    cb(undefined, new Mailbox(s, mailboxName, calls));
	}
    });
}




module.exports.tryMakeMailbox = tryMakeMailbox;

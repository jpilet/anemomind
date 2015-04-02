// A remote mailbox that we can play with, over HTTP.

var ServerConnection = require('./server-connection.js');
var mailboxCalls = require('./mailbox-calls.js');


function makeRpcCall(serverConnection, mailboxName, call) {
    return function() {
	serverConnection[call].apply(
	    null,
	    [mailboxName].concat(Array.prototype.slice.call(arguments))
	);
    };
}

function Mailbox(serverConnection, mailboxName, calls) {
    this.mailboxName = mailboxName;
    for (var i = 0; i < calls.length; i++) {
	var call = calls[i];

	// Add the mailbox name as the first argument.
	// We want talking with a remove mailbox to have
	// the same interface as a local one.
	this[call] = makeRpcCall(serverConnection, mailboxName, call);
    }
}


// Call this function when you need a new mailbox.
function tryMakeMailbox(serverAddress, userdata, mailboxName, cb) {
    var s0 = new ServerConnection(serverAddress);
    s0.login(userdata, function(err, s) {
	if (err) {
	    cb(err);
	} else {
	    // Register these as rpc calls.
	    s.registerCalls(mailboxCalls);
	    
	    cb(undefined, new Mailbox(s, mailboxName, mailboxCalls));
	}
    });
}




module.exports.tryMakeMailbox = tryMakeMailbox;

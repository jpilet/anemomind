/*
  All rpc functions should, by convention,
  deliver their results by calling a call-
  back.
*/  

// All RPC-bound functions should be fields of this object. Just add
// them here below.
var mb = require('../../../../mail/mail.sqlite.js');
var rpc = {};

function makeMailboxHandler(methodName) {
    return function() {
	var allArgs = Array.prototype.slice.call(arguments);
	var mailboxName = allArgs[0];
	var args = allArgs.slice(1);
	

	// Every mailbox has its own file
	var filename = mailboxName + '.mailsqlite.db';
	
	mb.tryMakeMailbox(
	    filename, mailboxName,
	    function(err, mailbox) {
		if (err) {
		    args[args.length-1](err);
		} else {
		    mailbox[methodName].apply(mailbox, args);
		}
	    }
	);
    }
}



// Just for testing
rpc.add = function(a, b, c, cb) {
    cb(undefined, a + b + c);
}



module.exports = rpc;

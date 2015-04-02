/*
  All rpc functions should, by convention,
  deliver their results by calling a call-
  back.
*/  

// All RPC-bound functions should be fields of this object. Just add
// them here below.
var mb = require('../../../../mail/mail.sqlite.js');
var calls = require('../../../../mail/mailbox-calls.js');
var assert = require('assert');
var rpc = {};

// A function that converts the RPC call (invisible to the user),
// where the mailbox name is passed as the first parameter,
// to a method call to a mailbox with that name.
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

for (var i = 0; i < calls.length; i++) {
    var call = calls[i];
    assert(typeof call == 'string');
    rpc[call] = makeMailboxHandler(call);
}




// Just for testing
rpc.add = function(a, b, c, cb) {
    cb(undefined, a + b + c);
}



module.exports = rpc;

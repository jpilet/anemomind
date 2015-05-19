// This file exposes an RPC interface over bluetooth
// to interact with a mailbox.

schema = require('mail/mailbox-schema.js');
rpcFuncTable = require('./rpcble').rpcFuncTable;
coder = require("mail/json-coder.js");
mb = require("mail/mail.sqlite.js");


// Conveniency function for
// error handling.
//
// If p evaluates to true, this function returns true without any side effects.
// Otherwise, it returns false and calls cb with the error message wrapped inside
// a JSON object.
function ensureCB(p, errorMessage, cb) {
    if (p) {
	return true;
    } else {
	cb({error: errorMessage});
	return false;
    }
}

/*
  TODO: Every time we access the mailbox, we open it, call the method and close it.
  If we want, we might want to leave the last opened mailbox opened until we call a
  method for a different mailbox.
*/
function callMailboxMethod(mailboxName, methodName, args, cb) {
    var filename = mailboxName + ".sqlite.db";
    mb.tryMakeMailbox(filename, mailboxName, function(err, mailbox) {
	if (err) {
	    cb(err);
	} else {
	    mailbox[methodName].apply(mailbox, args.concat([
		function(err, result) {
		    if (err) {
			cb(err);
		    } else {
			mb.close(function(err) {
			    if (err) {
				cb(err);
			    } else {
				cb(undefined, result);
			    }
			});
		    }
		}
	    ]));
	}
    });
}

function encodeResult(argSpec, result) {
    var len = argSpec.length;
    assert(len == 1 || len == 2);
    if (len == 1) {
	return {};
    } else {
	return {result: coder.encode(argSpec, result)};
    }
}

// Here we make a function that takes an incoming
// JSON object, decodes it, call a method on a local
// mailbox and return the result.
function makeRpcFunction(methodName, method) {
    return function(data, cb) {
	var mailboxName = data.thisMailboxName;
	try {
	    if (ensureCB(mailboxName != undefined,
			 "You must pass a mailbox name", cb)) {

		var args = coder.decodeArgs(method.input, data);
		callMailboxMethod(
		    mailboxName, methodName, args,
		    function(err, result) {
			if (err) {
			    var message = "Error accessing mailbox with name " +
				mailboxName + " and method " + methodName;
			    console.log(message);
			    console.log("The error is %j", err);
			    cb({error: message +  + ". See the server log for details."});
			} else {
			    cb(encodeResult(method.output, result));
			}
		    }
		);
	    }
	} catch (e) {
	    console.log("Caught this exception: %j", e);
	    cb({error: "Caught an exception on the server. See the server log for details."});
	}
    }
}

// Prefix all mailbox-related calls with mb
// to avoid naming collisions for common names (such as "reset")
function makeRpcFuncName(methodName) {
    return "mb_" + methodName;
}

// Here we register all the available mailbox calls
// that we serve, in the global variable
// rpcFuncTable.
for (var methodName in schema.methods) {
    var rpcFuncName = makeRpcFuncName(methodName);
    rpcFuncTable[rpcFuncName] = makeRpcFunction(
	methodName,
	schema.methods[methodName]
    );
}

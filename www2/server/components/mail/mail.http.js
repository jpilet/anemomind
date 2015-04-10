// A remote mailbox that we can play with, over HTTP.

var ServerConnection = require('./server-connection.js');
var schema = require('./mailbox-schema.js');
var coder = require('./json-coder.js');


// Make a method to put in the local mailbox object
// that will result in an HTTP request according
// to the schema.
function makeMethod(scon, mailboxName, methodName, methodSchema) {
    return function() {
	var allArgs = Array.prototype.slice.call(arguments);
	var lastArgIndex = allArgs.length - 1;
	var args = allArgs.slice(0, lastArgIndex);
	var cb = allArgs[lastArgIndex];
	var dataToPost = coder.encodeArgs(methodSchema.spec.input, args);
	dataToPost.thisMailbox = mailboxName;
	scon.makePostRequest(
	    dataToPost,
	    function(err, body) {
		if (err) {
		    cb(err);
		} else {
		    var output = methodSchema.spec.output;
		    var data = coder.decodeArgs(output, body);
		    if (data == undefined) {
			cb(new Error('Failed to decode HTTP response'));
		    } else {
			cb.apply(null, data);
		    }
		}
	    }
	);
    }
}

function Mailbox(serverConnection, mailboxName, calls) {
    this.mailboxName = mailboxName;
    for (methodName in schema.methods) {
	this[methodName] = makeMethod(
	    serverConnection,
	    mailboxName,
	    methodName,
	    schema.methods[methodName]
	);
    }
}


// Call this function when you need a new mailbox.
function tryMakeMailbox(serverAddress, userdata, mailboxName, cb) {
    var s = new ServerConnection(serverAddress);
    s.login(userdata, function(err) {
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

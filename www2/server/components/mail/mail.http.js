// A remote mailbox that we can play with, over HTTP.

var ServerConnection = require('./server-connection.js');
var coder = require('./json-coder.js');
var schema = require('./mailbox-schema.js');
var assert = require('assert');




// Make a method to put in the local mailbox object
// that will result in an HTTP request according
// to the schema.
function makeMethod(scon, mailboxName, method) {
    return function() {
	var allArgs = Array.prototype.slice.call(arguments);
	var lastArgIndex = allArgs.length - 1;
	var args = allArgs.slice(0, lastArgIndex);
	var cb = allArgs[lastArgIndex];

	var responseHandler = function(err, data) {
	    cb(coder.decode(method.output[0], err),
	       coder.decode(method.output[1], data));
	};

	if (method.httpMethod == 'post') {
	    scon.makePostRequest(
		mailboxName,
		method,
		coder.encodeArgs(method.input, args),
		responseHandler
	    );
	} else {
	    assert(args.length == method.input.length);
	    scon.makeGetRequest(
		mailboxName,
		method,
		coder.encodeGetArgs(method.input, args),
		responseHandler
	    );
	}
    }
}

function Mailbox(serverConnection, mailboxName) {
    this.mailboxName = mailboxName;
    for (methodName in schema.methods) {
	this[methodName] = makeMethod(
	    serverConnection,
	    mailboxName,
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
	    cb(undefined, new Mailbox(s, mailboxName));
	}
    });
}

module.exports.tryMakeMailbox = tryMakeMailbox;

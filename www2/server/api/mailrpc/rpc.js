/*
  All rpc functions should, by convention,
  deliver their results by calling a call-
  back.
*/  

var schema = require('mail/mailbox-schema.js');
var coder = require('mail/json-coder.js');
var assert = require('assert');
var mb = require('./mailbox.js');
var naming = require('mail/naming.js');

var boat = require('../boat/boat.controller.js');
var Boat = require('../boat/boat.model.js');
var boatAccess = require('../boat/access.js');

// All RPC-bound functions should be fields of this 'rpc' object. Just add
// them here below, using 'addRpc'.
//
// Every function should be on this form: function(user, args, cb),
//   where user is the req.user object, args are the arguments for the function,
//   and cb is a function that is called with the result upon completion.
var rpc = {};


function testUserAndMailbox(user, mailboxName) {
    if (user.email == "test@anemomind.com") {
	return mailboxName == "a" || mailboxName == "b" || mailboxName == "c";
    }
    return false;
}

// Check if a user is authorized to access a mailbox.
// If not, produce an error object.
function acquireMailboxAccess(user, mailboxName, cb) {
    if (testUserAndMailbox(user, mailboxName)) { // <-- for conveniency of unit testing.
	cb();
    } else {
	var errorObject = {statusCode: 401,
			   message: "Unauthorized access to " + mailboxName};
    	var parsed = naming.parseMailboxName(mailboxName);
	if (parsed == null) {
	    cb(errorObject);
	} else if (parsed.boatId) {
	    Boat.findById(parsed.boatId, function (err, boat) {
		if (err) {
		    // Don't the error details to intruders. Should we log it?
		    cb(errorObject);
		} else {
		    if (boat) {
			// I guess it makes sense to require write access.
			if (boatAccess.userCanWrite(user, boat)) {
			    cb(); // OK, move on
			} else {
			    cb(errorObject);
			}
		    } else { // No such boat.
			cb(errorObject);
		    }
		}
	    });
	} else {
	    cb(errorObject);
	}
    }
}

// This function is common, irrespective of whether it is a post or get request.
function callMailboxMethod(user, mailboxName, methodName, args, cb) {
    assert.notEqual(mb, undefined);
    assert.notEqual(mb.openMailbox, undefined);
    acquireMailboxAccess(
	user, mailboxName,
	function(err, p) {
	    if (err) {
		cb(err);
	    } else {
		mb.openMailbox(
		    mailboxName,
		    function(err, mailbox) {
			if (err) {
			    cb(err);
			} else {

			    try {
				mailbox[methodName].apply(
				    mailbox, args.concat([
					function(err, result) {
					    mailbox.close(
						function(err) {
						    cb(err, result);
						}
					    );
					}
				    ])
				);

			    // Ideally, all errors should be handled
		            // by passing them as the first argument
			    // to a callback, but having a catch here
			    // makes it safer.
			    } catch (e) { 
				console.log("Caught an exception when calling mailbox method: %j.", e);
				console.log("THIS SHOULD NOT HAPPEN and this is a bug. Please don't throw exceptions,");
				console.log("but pass the errors as the first argument to the callback instead.")
				cb(e);
			    }
			}
		    }
		);
	    }
	}
    );
}


////////////////////////////////////////////////////////////////////////////////
/// HTTP interface
////////////////////////////////////////////////////////////////////////////////
// http://stackoverflow.com/questions/18391212/is-it-not-possible-to-stringify-an-error-using-json-stringify
// Code that lets us serialize Error objects
// to be passed back as JSON over HTTP and
// handled by the client.
Object.defineProperty(Error.prototype, 'toJSON', {
    value: function () {
        var alt = {};

        Object.getOwnPropertyNames(this).forEach(function (key) {
            alt[key] = this[key];
        }, this);

        return alt;
    },
    configurable: true
});

function getStatusCode(err) {
    if (err) {
	if (typeof err == 'object') {
	    if (err.statusCode) {
		
		// In case of error, we can be specific about the status code.
		return err.statusCode;
	    }
	}
	
	// Default code for general, unspecified error:
	return 500;
    } else {
	return 200; // OK
    }
}

// This will handle an HTTP request related to a specific method.
function handler(method, req, res) {
    assert(method.httpMethod == 'post' || method.httpMethod == 'get');
    try {
	var resultCB = function(err, result) {
	    var code = getStatusCode(err);

	    // Do we need a try statement in this function?
	    if (err) {
		console.log('WARNING: There was an error on the server: %j', err);
		console.log('THE CODE IS %j', code);
	    }
	    
	    res.status(code).json(
		coder.encode(
		    method.output[err? 0 : 1], // How the return value should be coded.
		    (err? err : result) // What data to send.
		)
	    );
	};
	var mailboxName = req.params.mailboxName;
	var args = null;

	if (method.httpMethod == 'post') {
	    args = coder.decodeArgs(method.input, req.body);
	} else {
	    args = coder.decodeGetArgs(method.input, req.params);
	}
	
	callMailboxMethod(
	    req.user,
	    mailboxName,
	    method.name,
	    args,
	    resultCB
	);
    } catch (e) {
	resultCB(e);
    }
};

function makeHandler(method) {
    return function(req, res) {
	handler(method, req, res);
    };
}

// The basic path is just the method name together with the
// mailbox name.
function makeBasicSubpath(method) {
    return '/' + method.name + '/:mailboxName';
}

// This is a general method for both POST and GET request.
// For GET requests, it adds a pattern for the arguments.
function makeSubpath(method) {
    return makeBasicSubpath(method) +
	(method.httpMethod == 'post' || method.httpMethod == 'put'? '' :

	 // Also add a pattern for the arguments.
	 coder.makeGetArgPattern(method.input));
}

// Adds a route to the router for a method.
function bindMethodHandler(router, authenticator, method) {
    assert(schema.isValidHttpMethod(method.httpMethod));
    router[method.httpMethod](
	makeSubpath(method),
	authenticator,
	makeHandler(method)
    );
}

// Adds all routes to the router.
function bindMethodHandlers(router, authenticator) {
    // Register a GET or POST handler
    // for every remote function that
    // we can call.
    for (var methodName in schema.methods) {
	bindMethodHandler(router, authenticator, schema.methods[methodName]);
    }
}

// Prints a summary of the HTTP call to a remote function.
function makeMethodDesc(method) {
    console.log('Function name: %s', method.name);
    console.log('HTTP-call: %s %s\n', method.httpMethod.toUpperCase(), makeSubpath(method));
}

// To auto-generate a documentation.
function makeOverview() {
    for (var key in schema.methods) {
	makeMethodDesc(schema.methods[key]);
    }
}

module.exports.bindMethodHandlers = bindMethodHandlers;
module.exports.makeOverview = makeOverview;

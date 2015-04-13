/*
  All rpc functions should, by convention,
  deliver their results by calling a call-
  back.
*/  

var schema = require('../../components/mail/mailbox-schema.js');
var coder = require('../../components/mail/json-coder.js');
var assert = require('assert');
var mb = require('./mailbox.js');


// All RPC-bound functions should be fields of this 'rpc' object. Just add
// them here below, using 'addRpc'.
//
// Every function should be on this form: function(user, args, cb),
//   where user is the req.user object, args are the arguments for the function,
//   and cb is a function that is called with the result upon completion.
var rpc = {};


// Check if a user is authorized to access a mailbox.
function userCanAccess(user, mailboxName, cb) {
    var env = process.env.NODE_ENV;
    cb(undefined, (env == 'test' || env == 'development'));
}

// This function is common, irrespective of whether it is a post or get request.
function callMailboxMethod(user, mailboxName, methodName, args, cb) {
    assert(mb != undefined);
    assert(mb.openMailbox != undefined);
    userCanAccess(
	user, mailboxName,
	function(err, p) {
	    if (err) {
		cb(err);
	    } else if (!p) {
		cb(new Error(
		    'Unauthorized to access that mailbox with name '
			+ mailboxName
		));
	    } else {
		mb.openMailbox(
		    mailboxName,
		    function(err, mailbox) {
			if (err) {
			    cb(err);
			} else {
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
/* TODO:
   
   * Protect this interface using the authentication
   
*/   

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


function handler(method, req, res) {
    assert(method.httpMethod == 'post' || method.httpMethod == 'get');
    try {
	var resultCB = function(err, result) {
	    // Do we need a try statement in this function?
	    if (err) {
		console.log('WARNING: There was an error on the server: %j', err);
	    }
	    var statusCode = (err? 500 : 200);
	    res.status(statusCode).json(
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

function makeBasicSubpath(method) {
    return '/' + method.name + '/:mailboxName';
}

function makeSubpath(method) {
    return makeBasicSubpath(method) +
	(method.httpMethod == 'post' || method.httpMethod == 'put'? '' :

	 // Also add a pattern for the arguments.
	 coder.makeGetArgPattern(method.input));
}

function bindMethodHandler(router, authenticator, method) {
    var httpMethod = method.httpMethod;

    if (method.httpMethod == 'post') {
	router.post(
	    makeSubpath(method),
	    authenticator,
	    makeHandler(method)
	);
    } else {
	assert(method.httpMethod == 'get');
	router.get(
	    makeSubpath(method),
	    authenticator,
	    makeHandler(method)
	);
    }
}


function bindMethodHandlers(router, authenticator) {
    // Register a GET or POST handler
    // for every remote function that
    // we can call.
    for (var methodName in schema.methods) {
	bindMethodHandler(router, authenticator, schema.methods[methodName]);
    }
}

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


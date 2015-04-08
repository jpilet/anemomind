/*
  All rpc functions should, by convention,
  deliver their results by calling a call-
  back.
*/  

var calls = require('../../components/mail/mailbox-calls.js');
var assert = require('assert');
var JSONB = require('json-buffer');
var mb = require('./mailbox.js');


// All RPC-bound functions should be fields of this 'rpc' object. Just add
// them here below, using 'addRpc'.
//
// Every function should be on this form: function(user, args, cb),
//   where user is the req.user object, args are the arguments for the function,
//   and cb is a function that is called with the result upon completion.
var rpc = {};


// Utility function that should be used when adding
// functions to the rpc object.
function addRpc(dstObj, name, fn) {
    assert(typeof dstObj == 'object');
    assert(typeof name == 'string');
    assert(typeof fn == 'function');

    // Make sure that there are no naming collisions when we map to lower case.
    var namelow = name.toLowerCase();
    assert(dstObj[namelow] == undefined);
    dstObj[namelow] = fn;
}

// Check if a user is authorized to access a mailbox.
function userCanAccess(user, mailboxName, cb) {
    var env = process.env.NODE_ENV;
    cb(undefined, (env == 'test' || env == 'development'));
}

// A function that converts the RPC call (invisible to the user),
// where the mailbox name is passed as the first parameter,
// to a method call to a mailbox with that name.
function makeMailboxHandler(methodName) {
    return function(user, allArgs, cb) {
	// How to turn an arguments map into an array:  Array.prototype.slice.call(arguments);
	var mailboxName = allArgs[0];

	var args = allArgs.slice(1, allArgs.length);

	// Every mailbox has its own file

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
}





for (var i = 0; i < calls.length; i++) {
    var call = calls[i];
    assert(typeof call == 'string');
    addRpc(rpc, call, makeMailboxHandler(call));
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

function handler(req, res) {
    try {
	var resultCB = function(err, result) {
	    res.json(200, {
		err: JSONB.stringify(err),
		result: JSONB.stringify(result)
	    });
	};

	var args = JSONB.parse(req.body.args);


	// The arguments passed to the function that we are calling:
	//   * The rest of the arguments sent by json
	//   * A callback for the result.

	var fnName = req.params[0];
	var fn = rpc[fnName.toLowerCase()];

	if (fn == undefined) {
	    resultCB(
		{
		    noSuchFunction: fnName,
		    availableFunctions: Object.keys(rpc)
		}
	    );
	} else {
	    fn(req.user, args, resultCB);
	}
    } catch (e) {
	resultCB(e);
    }
};


module.exports = handler;

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

function handler(req, res) {
    try {
	var resultCB = function(err, result) {
	    res.json(
		200,
		coder.encodeArgs(
		    schema.spec.output, [err, result]
		)
	    );
	};

	var methodName = req.params.methodName;
	var mailboxName = req.params.mailboxName;
	
	var args = coder.decodeArgs(schema.methods[methodName].spec.input, req.body);
	if (typeof fnName == 'string') {
	    callMailboxMethod(
		req.user,
		mailboxName,
		methodName,
		args,
		resultCB
	    );
	} else {
	    resultCB('The function name should be a string, but got '
		     + fnName);
	}
    } catch (e) {
	resultCB(e);
    }
};


module.exports = handler;

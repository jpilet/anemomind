/*
  All rpc functions should, by convention,
  deliver their results by calling a call-
  back.
*/  

var mb = require('../../components/mail/mail.sqlite.js');
var calls = require('../../components/mail/mailbox-calls.js');
var assert = require('assert');
var JSONB = require('json-buffer');


// All RPC-bound functions should be fields of this 'rpc' object. Just add
// them here below.
var rpc = {};

// A function that converts the RPC call (invisible to the user),
// where the mailbox name is passed as the first parameter,
// to a method call to a mailbox with that name.
function makeMailboxHandler(methodName) {
    return function() {
	var allArgs = Array.prototype.slice.call(arguments);
	var mailboxName = allArgs[0];

	var args = allArgs.slice(1, allArgs.length-1);
	var cb = allArgs[allArgs.length-1];

	// Every mailbox has its own file
	var filename = mailboxName + '.mailsqlite.db';
	
	mb.tryMakeMailbox(
	    filename, mailboxName,
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





for (var i = 0; i < calls.length; i++) {
    var call = calls[i];
    assert(typeof call == 'string');
    rpc[call] = makeMailboxHandler(call);
}




// Just for testing
rpc.add = function(a, b, c, cb) {
    cb(undefined, a + b + c);
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
    var args = JSONB.parse(req.body.args);
    var resultCB = function(err, result) {
	res.json(201, {
	    err: JSONB.stringify(err),
	    result: JSONB.stringify(result)
	});
    };

    // The arguments passed to the function that we are calling:
    //   * The rest of the arguments sent by json
    //   * A callback for the result.
    var argArray = args.concat([resultCB]);

    var fn = rpc[req.body.fn];

    if (fn == undefined) {
	resultCB(
	    {
		noSuchFunction: req.body.fn,
		availableFunctions: Object.keys(rpc)
	    }
	);
    } else {
	try {
	    fn.apply(null, argArray);
	} catch (e) {
	    console.log('Caught an exception while processing RPC call: %j', e);
	    resultCB(e);
	}
    }
};

function handleError(res, err) {
    res.send(500, err);
}



module.exports = handler;

// A simulation of mailbox synchronizations
var mb = require("./mail.http.js");
var async = require("async");
var demo = require('./syncdemo-3.js');
var assert = require('assert');

var testuser = {
    'email': 'kalle@abc.com',
    'password': 'abc'
};

var address = 'http://localhost:9000';

var boxnames = ["a", "b", "c"];


console.log('Try to synchronize three mailboxes');
async.map(
    boxnames,
    function (boxname, cb) {
	mb.tryMakeMailbox(
	    address, testuser, boxname,
	    function(err, mailbox) {
		assert(err == undefined);

		// Always reset first so that we start from
		// the same state.
		mailbox.reset(function(err) {
		    cb(err, mailbox);
		});
	    }
	);
    },
    function(err, boxes) {
	assert(err == undefined);
	demo.synchronizeThreeMailboxes(boxes, function(err) {
	    if (err) {
		console.log('THERE WAS AN ERROR: %j', err);
		console.log(
		    'Please make sure that there is a user like this on the server: %j',
		    testuser
		);
	    } else {
		console.log('SUCCESS!!!');
	    }
	});
    }
);



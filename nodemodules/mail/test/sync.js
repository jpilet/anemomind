// A simulation of mailbox synchronizations
var mb = require("../mail.sqlite.js");
var async = require("async");
var demo = require('../syncdemo-3.js');
var assert = require('assert');

var boxnames = ["a", "b", "c"];
// Main call to run the demo
describe(
    'Synchronize',
    function() {
	it(
	    'Synchronize three mailboxes with each other',
	    function(done) {

		this.timeout(4000);

		async.map(
		    boxnames,
		    function (boxname, cb) {
			mb.tryMakeMailbox(":memory:", boxname, cb);
		    },
		    function(err, boxes) {
			assert.equal(err, undefined);
			demo.synchronizeThreeMailboxes(boxes, done);
		    }
		);
	    }
	);
    }
);



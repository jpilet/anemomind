var assert = require('assert');
var naming = require('../naming.js');


it(
    'naming',
    function() {
	describe(
	    'Should make a mailbox name from a boat id and parse it',
	    function() {
		var boatId = 119;
		var name = naming.makeMailboxNameFromBoatId(boatId);
		assert.equal(name, "boatId_119");
		var parsed = naming.parseMailboxName(name);
		console.log("parsed = %j", parsed);
		assert.equal(parsed.boatId, boatId);
	    });
    });

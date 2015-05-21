var assert = require('assert');
var naming = require('../naming.js');


it(
  'naming',
  function() {
    describe(
      'Should make a mailbox name from a boat id and parse it',
      function() {
	var boatId = "119";
	var name = naming.makeMailboxNameFromBoatId(boatId);
	assert.equal(name, "boatId_119");
	var parsed = naming.parseMailboxName(name);
	assert.equal(parsed.boatId, boatId);
      });

    describe(
      'Should make a mailbox name from a box id and parse it',
      function() {
	var boxId = "abc-119";
	var name = naming.makeMailboxNameFromBoxId(boxId);
	assert.equal(name, "boxId_abc-119");
	var parsed = naming.parseMailboxName(name);
	assert.equal(parsed.boxId, boxId);
      });
  });



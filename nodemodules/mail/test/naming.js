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
	assert.equal(name, "boat119");
	var parsed = naming.parseMailboxName(name);
	assert.equal(parsed.prefix, "boat");
	assert.equal(parsed.id, boatId);
      });

    describe(
      'Should make a mailbox name from a box id and parse it',
      function() {
	var boxId = "abc-119";
	var name = naming.makeMailboxNameFromBoxId(boxId);
	assert.equal(name, "boxabc-119");
	var parsed = naming.parseMailboxName(name);
	assert.equal(parsed.prefix, "box");
	assert.equal(parsed.id, boxId);
      });

    describe('Should compose and decompose a filename', function() {
      var mailboxName = 'rulle';
      assert.equal(mailboxName, naming.getMailboxNameFromFilename(
        naming.makeDBFilename(mailboxName)
      ));
    });
  });



var lmb = require('../components/LocalMailbox.js');
var assert = require('assert');

it('LocalMailbox', function() {
  describe(
    'Instantiate a local mailbox and reset it',
    function(done) {
      lmb.setMailRoot('/tmp/localmail/');
      lmb.open(function(err, mb) {
	assert.equal(err, undefined);
	assert(mb);
	mb.reset(function(err) {
	  assert.equal(err, undefined);
	  done();
	});
      });
    }
  );
});

var lmb = require('../components/LocalMailbox.js');
var assert = require('assert');

describe('LocalMailbox', function() {
  it(
    'Should instantiate a local mailbox and reset it',
    function(done) {
      lmb.setMailRoot('/home/jonas/');
      lmb.open(function(err, mb) {
	assert.equal(err, undefined);
	assert(mb);
	mb.reset(function(err) {
	  assert.equal(err, undefined);
	  mb.sendPacket('rulle', 122, new Buffer(0), function(err) {
	    assert.equal(err, undefined);
	    mb.close(function(err) {
	      assert.equal(err, undefined);
	      console.log('DONE!!!!!');
	      done();
	    });
	  });
	});
      });
    }
  );
});





// it('LocalMailbox', function() {
//   describe(
//     'get mailbox name',
//     function(done) {
//       console.log('done = %j', done);
//       lmb.getName(function(name) {
// 	console.log('The name is ' + name);
// 	done();
//       });
//     });
// });

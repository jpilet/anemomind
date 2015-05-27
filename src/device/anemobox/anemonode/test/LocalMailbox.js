var lmb = require('../components/LocalMailbox.js');
var assert = require('assert');
var fs = require('fs');

describe('LocalMailbox', function() {
  it(
    'Should instantiate a local mailbox and reset it',
    function(done) {
      lmb.setMailRoot('/tmp/anemobox/');
      lmb.open(function(err, mb) {
	assert.equal(err, undefined);
	assert(mb);
	mb.reset(function(err) {
	  assert.equal(err, undefined);
	  mb.sendPacket('rulle', 122, new Buffer(0), function(err) {
	    assert.equal(err, undefined);
	    mb.getTotalPacketCount(function(err, n) {
	      assert.equal(n, 1);
	      assert.equal(err, undefined);
	      mb.close(function(err) {
		assert.equal(err, undefined);
		done();
	      });
	    });
	  });
	});
      });
    }
  );

  it('Post a log file', function(done) {
    lmb.setMailRoot('/tmp/anemobox/');
    fs.writeFile('/tmp/anemolog.txt', 'This is a log file', function(err) {
      assert(!err);
      lmb.open(function(err, mb) {
        assert(!err);
        mb.reset(function(err) {
          assert(!err);
          mb.close(function(err) {
            assert(!err);
            lmb.postLogFile('/tmp/anemolog.txt', function(err) {
              assert(!err);
              mb.open(function(err, mb) {
                assert(!err);
                mb.getAllPackets(function(err, packets) {
                  assert(packets.length == 1);
                  done();
                });
              });
            });
          });
        });
      });
    });
  });
});

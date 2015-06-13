// A simulation of mailbox synchronizations
var mb = require("../mail.sqlite.js");
var async = require("async");
var demo = require('../syncdemo-3.js');
var assert = require('assert');
var sync = require('../sync.js');

var boxnames = ["a", "b", "c"];
// Main call to run the demo


function makeAndReset(name, cb) {
  mb.tryMakeMailbox(
    '/tmp/testbox' + name + '.db',
    name, function(err, mailbox) {
      mailbox.forwardPackets = true;
      if (err) {
        cb(err);
      } else {
        mailbox.reset(function(err) {
          if (err) {
            cb(err);
          } else {
            cb(null, mailbox);
          }
        });
      }
    });
}


describe('Synchronize', function() {
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
	});
    });
  
  it('propbug', function(done) {
    makeAndReset('a', function(err, a) {
      assert(!err);
      makeAndReset('b', function(err, b) {
        assert(!err);
        makeAndReset('a2', function(err, a2) {
          assert(!err);
          a.sendPacket('b', 129, new Buffer(3), function(err) {
            assert(!err);
            sync.synchronize(a, b, function(err) {
              assert(!err);
              b.getTotalPacketCount(function(err, count) {
                assert(!err);
                assert.equal(count, 1);
                sync.synchronize(a2, b, function(err) {
                  assert(!err);
                  a2.getTotalPacketCount(function(err, count) {
                    assert(!err);
                    assert.equal(count, 0); // Before the bug fix, this would have failed.
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
});



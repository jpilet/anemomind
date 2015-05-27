var mb = require('mail/mail.sqlite.js');
var lmb = require('../components/LocalMailbox.js');
var config = require('../components/config.js');
var ensureConfig = require('./EnsureConfig.js');
var fs = require('fs');
var sync = require('mail/sync.js');
var assert = require('assert');

function makeLogFilename(index) {
  return "/tmp/cleanuptest" + index + ".txt";
}

function createAndSendLogFiles(count, cb) {
  if (count == 0) {
    cb(null, mb);
  } else {
    var contents = "This is log file " + count;
    var filename = makeLogFilename(count);
    fs.writeFile(filename, contents, function(err) {
      if (err) {
        cb(err);
      } else {
        lmb.postLogFile(filename, function(err) {
          if (err) {
            cb(err);
          } else {
            createAndSendLogFiles(count-1, cb);
          }
        });
      }
    });
  }
}

function countLogFiles(n, counter, cb) {
  if (n == 0) {
    cb(null, counter);
  } else {
    var filename = makeLogFilename(n);
    fs.readFile(filename, function(err, data) {
      countLogFiles(n-1, counter + (err? 0 : 1), cb);
    });
  }
}

function resetLocalMailbox(cb) {
  lmb.open(function(err, local) {
    assert(!err);
    local.reset(function(err) {
      assert(!err);
      local.close(cb);
    });
  })
}

describe('Cleanup sent log files', function() {
  it('cleanup', function(done) {
    ensureConfig(function(err, cfg) {
      lmb.setRemoveLogFiles(true);
      lmb.getServerSideMailboxName(function(err, mailboxName) {
        assert(!err);
        resetLocalMailbox(function(err) {
          createAndSendLogFiles(5, function(err, localMailbox) {
            assert(!err);
            mb.tryMakeMailbox('/tmp/serverbox.db', mailboxName, function(err, serverMailbox) {
              serverMailbox.reset(function(err) {
                serverMailbox.ackFrequency = 3;
                assert(!err);
                lmb.open(function(err, anemoboxMailbox) {
                  assert(!err);
                  sync.synchronize(anemoboxMailbox, serverMailbox, function(err) {
                    assert(!err);

                    // Synchronize a second time, to make sure that any ack
                    // packets emitted on the
                    // server are moved back.
                    sync.synchronize(anemoboxMailbox, serverMailbox, function(err) {
                      assert(!err);
                      countLogFiles(5, 0, function(err, remainingCount) {
                        assert(!err);
                        assert(remainingCount == 2);
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
  });
});

var mb = require('mail/mail.sqlite.js');
var lmb = require('../components/LocalMailbox.js');
var config = require('../components/config.js');
var ensureConfig = require('./EnsureConfig.js');
var fs = require('fs');

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


describe('Cleanup sent log files', function() {
  it('Should cleanup log files once they have been delivered', function(done) {
    ensureConfig(function(err, cfg) {
      lmb.getServerSideMailboxName(function(err, mailboxName) {
        createAndSendLogFiles(5, function(err, localMailbox) {
          done();
        });
      });
    });
  });
});

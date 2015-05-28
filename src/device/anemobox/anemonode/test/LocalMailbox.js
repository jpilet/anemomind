var lmb = require('../components/LocalMailbox.js');
var assert = require('assert');
var fs = require('fs');
var file = require('mail/file.js');
var ensureConfig = require('./EnsureConfig.js');
var mkdirp = require('mkdirp');

describe('LocalMailbox', function() {
  it(
    'Should instantiate a local mailbox and reset it',
    function(done) {
      ensureConfig(function(err, cfg) {
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
      });
    }
  );

  it('Post a log file', function(done) {
    ensureConfig(function(err, cfg) {
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
                lmb.open(function(err, mb) {
                  assert(!err);
                  mb.getAllPackets(function(err, packets) {
                    assert(packets.length == 1);
                    var packet = packets[0];
                    var msg = file.unpackFileMessage(packet.data);
                    assert(file.isLogFileInfo(msg.info));
                    var filedata = msg.data;
                    fs.readFile('/tmp/anemolog.txt', function(err2, filedata2) {
                      assert(filedata2 instanceof Buffer);
                      assert.equal(filedata.length, filedata2.length);
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



var testLogRoot = '/tmp/testlogs/';

function makeLogFilename(i) {
  return testLogRoot + 'testlog' + i + '.txt';
}

function makeLogFileContents(i) {
  return 'LogFile' + i;
}

function createAndPostLogFiles(postFilterFun, n, cb) {
  if (n == 0) {
    cb();
  } else {
    var index = n-1;
    var fname = makeLogFilename(index);
    fs.writeFile(fname, makeLogFileContents(i), function(err) {
      if (err) {
        cb(err);
      } else {
        var next = function() {createAndPostLogFiles(n - 1, cb);}
        if (postFilterFun(index)) {
          lmb.postLogFile(fname, function(err) {
            if (err) {
              cb(err);
            } else {
              next();
            }
          });
        } else {
          next();
        }
      }
    });
  }
}


describe('Listing files not posted', function() {
  
});
  
  mkdirp(testLogRoot, function(err) {
    var odd = function(i) {return i % 2 == 0;}
    createAndPostLogFiles(odd, 7, function(err) {

    });
  });


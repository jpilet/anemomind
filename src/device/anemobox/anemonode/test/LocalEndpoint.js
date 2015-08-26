var lmb = require('../components/LocalEndpoint.js');
var assert = require('assert');
var fs = require('fs');
var file = require('endpoint/logfile.js');
var ensureConfig = require('./EnsureConfig.js');
var mkdirp = require('mkdirp');
var Q = require('q');
var config = require('../components/config.js');
var mail2 = require('endpoint/endpoint.sqlite.js');

describe('LocalEndpoint', function() {
  it(
    'Should instantiate a local endpoint and reset it',
    function(done) {
      ensureConfig(function(err, cfg) {
        lmb.setMailRoot('/tmp/anemobox/');


        
        lmb.withLocalEndpoint(function(mb, doneMB) {
	  assert.equal(err, undefined);
	  assert(mb);
	  mb.reset(function(err) {
	    assert.equal(err, undefined);
	    mb.sendPacket('rulle', 122, new Buffer(0), function(err) {
	      assert.equal(err, undefined);
	      mb.getTotalPacketCount(function(err, n) {
	        assert.equal(n, 1);
	        assert.equal(err, undefined);
                doneMB();
              });
            });
          });
        }, done);
      });
    }
  );


  it(
    'Should fail to instantiate a local endpoint due to empty name',
    function(done) {
      ensureConfig(function(err, cfg) {
        lmb.setMailRoot('/tmp/anemobox/');
        var operationPerformed = false;
        lmb.withNamedLocalEndpoint("", function(mb, doneMB) {
          operationPerformed = true;
        }, function(err) {
          assert(err); // <-- It is an error to instantiate a local endpoint with empty name.
          assert(!operationPerformed);
          done();
        });
      });
    });


  it('Post a log file', function(done) {
    ensureConfig(function(err, cfg) {
      lmb.setMailRoot('/tmp/anemobox/');
      fs.writeFile('/tmp/anemolog.txt', 'This is a log file', function(err) {
        assert(!err);
        
        lmb.withLocalEndpoint(function(mb, doneMB) {
          assert(!err);
          mb.reset(function(err) {
            assert(!err);
            mb.close(function(err) {
              assert(!err);
              lmb.postLogFile('/tmp/anemolog.txt', function(err) {
                assert(!err);
                
                lmb.withLocalEndpoint(function(mb, doneMB2) {
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
                      doneMB2();
                    });
                  });
                }, doneMB);
              });
            });
          });
        }, done());
      });
    });
  });
});



var testLogRoot = '/tmp/testlogsMissing/';

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
    fs.writeFile(fname, makeLogFileContents(index), function(err) {
      if (err) {
        cb(err);
      } else {
        var next = function() {createAndPostLogFiles(postFilterFun, index, cb);}
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

function preparePostingTest(cb) {
  mkdirp(testLogRoot, function(err) {
    var even = function(i) {return i % 2 == 0;}
    lmb.reset(function(err) {
      createAndPostLogFiles(even, 7, cb);
    });
  });
}

describe('Listing and posting files not posted', function() {
  it('Post log files', function(done) {
    preparePostingTest(function(err) {
      console.log(err);
      assert(!err);
      lmb.listLogFilesNotPosted(testLogRoot, function(err, files) {
        assert(!err);
        assert(files.length == 3);
        lmb.postRemainingLogFiles(testLogRoot, function(err) {
          assert(!err);
          lmb.listLogFilesNotPosted(testLogRoot, function(err, files) {
            assert(!err);
            assert(files.length == 0);
            done();
          });
        });
      });
    });
  });
});

  


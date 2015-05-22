var assert = require('assert');
var mb = require('../mail.sqlite.js');
var fs = require('fs');

function makeLogFilename(index) {
  return '/tmp/anemolog' + index + '.log';
}


function makeLogFile(index, cb) {
  var filename = makeLogFilename(index);
  var contents = 'This is log file ' + index;
  fs.writeFile(filename, contents, cb);
}

function logFileExists(index, cb) {
  fs.exists(makeLogFilename(index), cb);
}

function logFilesExistSub(dst, count, cb) {
  if (count == 0) {
    cb(dst);
  } else {
    var index = count-1;
    logFileExists(index, function(p) {
      dst[index] = p;
      logFilesExistSub(dst, index, cb);
    });
  }
}

function logFilesExist(count, cb) {
  logFilesExistSub(new Array(count), count, cb);
}


function makeLogFiles(count, cb) {
  if (count == 0) {
    cb();
  } else {
    var index = count-1;
    makeLogFile(index, function(err) {
      if (err) {
	cb(err);
      } else {
	makeLogFiles(index, cb);
      }
    });
  }
}

function removeLogFile(index, cb) {
  fs.unlink(makeLogFilename(index), cb);
}

function validRemoveError(err) {
  if (err) {
    if (err.code == "ENOENT") {
      return false;
    }
    return true;
  }
  return false;
}
"ENOENT"

function removeLogFiles(count, cb) {
  if (count == 0) {
    cb();
  } else {
    var index = count-1;
    removeLogFile(index, function(err) {
      if (validRemoveError(err)) {
	cb(err);
      } else {
	removeLogFiles(index, cb);
      }
    });
  }
}

function makeEndpoint(name, cb) {
  mb.tryMakeMailbox('/tmp/' + name + '.db', name, cb);
}

function makeEndpointsSub(dst, index, arr, cb) {
  if (index == arr.length) {
    cb(null, arr);
  } else {
    makeEndpoint(arr[index], function(err, mailbox) {
      if (err) {
	cb(err);
      } else {
	dst[index] = mailbox;
	makeEndpointsSub(dst, index+1, arr, cb);
      }
    });
  }
}

function makeEndpoints(arr, cb) {
  makeEndpointsSub(new Array(arr.length), 0, arr, cb);
}

function makeAnemoSetup(cb) {
  var names = ["server", "phone", "box"];
  makeEndpoints(names, cb);
}


describe(
  'Test utilities',
  function() {
    it(
      'Make sure that the utilities for testing log files work',
      function(done) {
	makeLogFiles(35, function(err) {
	  assert(!err);
	  logFilesExist(35, function(arr) {
	    assert.equal(arr.length, 35);
	    for (var i = 0; i < arr.length; i++) {
	      assert(arr[i]);
	    }
	    removeLogFiles(35, function(err) {
	      assert(!err);
	      logFilesExist(35, function(arr) {
		for (var i = 0; i < arr.length; i++) {
		  assert(!arr[i]);
		}
		done();
	      });
	    });
	  });
	})
      });
    });

describe('anemo setup', function() {
  it('Should instantiate 3 mailboxes on the file system', function(done) {
    makeAnemoSetup(function(err, boxes) {
      assert(!err);
      assert.equal(boxes.length, 3);
      done();
    })
  });
});

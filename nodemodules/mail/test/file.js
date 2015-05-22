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

describe(
  'Sending files',
  function() {
    it(
      'Synchronize a lot of log files, and remove the log files once they have arrived.',
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

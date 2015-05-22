var assert = require('assert');
var file = require('../file.js');
var mb = require('../mail.sqlite.js');
var fs = require('fs');
var Q = require('q');
var sync = require('../sync.js');

function makeLogFilename(index) {
  return '/tmp/anemolog' + index + '.log';
}

function makeLogData(index) {
  return 'This is log file ' + index;
}


function makeLogFile(index, cb) {
  var filename = makeLogFilename(index);
  var contents = makeLogData(index);
  fs.writeFile(filename, contents, cb);
}

function logFileExists(index, cb) {
  fs.exists(makeLogFilename(index), cb);
}

function sendFiles(src, dst, count, cb) {
  if (count == 0) {
    cb();
  } else {
    var index = count-1;
    file.sendFile(src, dst, makeLogFilename(index), {logIndex: index}, function(err) {
      if (err) {
	cb(err);
      } else {
	sendFiles(src, dst, index, cb);
      }
    });
  }
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
  mb.tryMakeMailbox('/tmp/' + name + '.db', name, function(err, mb) {
    if (err) {
      cb(err);
    } else {
      mb.reset(function(err) {
	cb(err, mb);
      });
    }
  });
}

function makeEndpointsSub(dst, index, arr, cb) {
  if (index == arr.length) {
    cb(null, dst);
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

function isObjectWithFields(x, fields) {
  if (typeof x == 'object') {
    for (var i = 0; i < fields.length; i++) {
      if (!x[files[i]]) {
	return false;
      }
      return true;
    }
  }
  return false;
}

function all(x) {
  for (var i = 0; i < x.length; i++) {
    if (!x[i]) {
      return false;
    }
  }
  return true;
}

// Called when the server receives a packet
function makeServerPacketHandler(markArray, deferred) {
  return function(mailbox, packet) {
    if (packet.label == common.file) {
      var msg = msgpack.unpack(packet.data);
      if (isObjectWithFields(msg.info, ['logIndex'])) {
	var index = msg.info.logIndex;
	var msgText = msg.data.toString('utf8');
	if (msgText == makeLogData(index)) {
	  markArray[index] = true;
	  if (all(markArray)) {
	    promise.resolve(markArray);
	  }
	} else {
	  promise.reject('Message data mismatch');
	}
      }
    }
  }
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

describe('log file sync', function() {
  it('Should synchronize log files', function(done) {
    makeAnemoSetup(function(err, boxes) {
      assert(!err);
      assert.equal(boxes.length, 3);
      
      var server = boxes[0];
      var phone = boxes[1];
      var box = boxes[2];
      
      assert(server.mailboxName == 'server');
      assert(phone.mailboxName == 'phone');
      assert(box.mailboxName == 'box');

      var serverDeferred = Q.defer();
      server.onPacketReceived = makeServerPacketHandler(new Array(35), serverDeferred);

      // Create the log files
      makeLogFiles(3, function(err) {
	assert(!err);
	sendFiles(box, 'server', 3, function(err) {
	  assert(!err);
	  
	  mb.dispAllTableData(box.db, function(err) {
	    mb.dispAllTableData(phone.db, function(err) {
	      done();
	    });
	  });


	  // sync.synchronize(box, phone, function(err) {
	  //   assert(!err);
	  //   done();
	  //   // sync.synchronize(phone, server, function(err) {
	  //   //   assert(!err);
	  //   //   done();
	  //   // });
	  // });
	  
	});
      });
    })
  });
});

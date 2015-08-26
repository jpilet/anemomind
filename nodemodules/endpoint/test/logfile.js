var assert = require('assert');
var file = require('../logfile.js');
var mb = require('../endpoint.sqlite.js');
var fs = require('fs');
var Q = require('q');
var common = require('../common.js');
var sync = require('../sync2.js');

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
    file.sendFile(
      src, dst, makeLogFilename(index), {logIndex: index}, common.logfile,
      function(err) {
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
  mb.tryMakeEndpoint('/tmp/' + name + '.db', name, function(err, mb) {
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
    makeEndpoint(arr[index], function(err, endpoint) {
      if (err) {
	cb(err);
      } else {
	dst[index] = endpoint;
	makeEndpointsSub(dst, index+1, arr, cb);
      }
    });
  }
}

function makeEndpoints(arr, cb) {
  makeEndpointsSub(new Array(arr.length), 0, arr, cb);
}


function all(x) {
  for (var i = 0; i < x.length; i++) {
    if (!x[i]) {
      return false;
    }
  }
  return true;
}

function isLogFileMsg(msg) {
  return common.isObjectWithFields(msg, ['info', 'path', 'data']);
}

function countMarked(arr) {
  var counter = 0;
  for (var i = 0; i < arr.length; i++) {
    if (arr[i]) {
      counter++;
    }
  }
  return counter;
}


// Called when the server receives a packet
function makeServerPacketHandler(markArray, deferred) {
  return function(endpoint, packet) {
    if (packet.label == common.logfile) {
      var msg = file.unpackFileMessage(packet.data);
      if (isLogFileMsg(msg)) {
	var index = msg.info.logIndex;
	var msgText = msg.data.toString('utf8');
	if (msgText == makeLogData(index)) {
	  markArray[index] = true;
	  if (all(markArray)) {
	    deferred.resolve(markArray);
	  }
	} else {
	  deferred.reject('Message data mismatch');
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
	makeLogFiles(3, function(err) {
	  assert(!err);
	  logFilesExist(3, function(arr) {
	    assert.equal(arr.length, 3);
	    for (var i = 0; i < arr.length; i++) {
	      assert(arr[i]);
	    }
	    removeLogFiles(3, function(err) {
	      assert(!err);
	      logFilesExist(3, function(arr) {
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

describe('logfiles', function() {
  this.timeout(12000);
  it('Should synchronize log files', function(done) {
    makeAnemoSetup(function(err, boxes) {
      assert(!err);
      assert.equal(boxes.length, 3);
      
      var server = boxes[0];
      var phone = boxes[1];
      var box = boxes[2];
      
      assert(server.name == 'server');
      assert(phone.name == 'phone');
      assert(box.name == 'box');

      var n = 5;
      var serverDeferred = Q.defer();

      server.ackFrequency = 3;

      server.addPacketHandler(makeServerPacketHandler(
	new Array(n), serverDeferred));

      // Create the log files
      makeLogFiles(n, function(err) {
	assert(!err);
	sendFiles(box, 'server', n, function(err) {
	  assert(!err);
	  sync.synchronize(box, phone, function(err) {
	    assert(!err);
	    sync.synchronize(phone, server, function(err) {
	      assert(!err);

	      // Wait for the server to receive all the log files
	      // sent from the box
	       serverDeferred.promise.then(function(value) {
	         assert(all(value));

		 // By now, the server will have emitted an ack packet for 3 of the packets.
		 // That ack packet is propagated back.
		 sync.synchronize(phone, server, function(err) {
		   assert(!err);
		   sync.synchronize(box, phone, function(err) {
		     done();
		   });
		 });
	       });
	    });
	  });
	});
      });
    })
  });
});


// Investigate this:
// http://stackoverflow.com/questions/13924936/encoding-messagepack-objects-containing-node-js-buffers
describe(
  'Packing and unpacking files',
  function() {
    it('Should pack and unpack a file', function(done) {
      var filename = '/tmp/mailtestfile.txt';
      var txtdata = 'This is a file containing text.';
      fs.writeFile(filename, txtdata, function(err) {
        file.readAndPackFile(filename, file.makeLogFileInfo(), function(err, packed) {
          var unpacked = file.unpackFileMessage(packed);
          fs.readFile(filename, function(err, filedata2) {
            // Would have failed with 'msgpack'. But works with 'msgpack-js'
            assert(unpacked.data instanceof Buffer); 
            assert(filedata2 instanceof Buffer);
            assert(unpacked.data.equals(filedata2));
            done();
          });
        });
      });
    });
  });

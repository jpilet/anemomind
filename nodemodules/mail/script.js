var common = require('./common.js');
var msgpack = require('msgpack-js');
var bigint = require('./bigint.js');
var fs = require('fs');
var exec = require('child_process').exec;

function validScriptType(type) {
  return type == "sh" || type == "js";
}

function packScriptRequest(type, script, reqCode) {
  var x = {type:type, script:script};
  if (reqCode) {
    x.reqCode = reqCode;
  }
  return msgpack.encode(x);
}

function unpackScriptRequest(x) {
  return msgpack.decode(x);
}

function packScriptResponse(x) {
  return msgpack.encode(x);
}

function unpackScriptResponse(x) {
  return msgpack.decode(x);
}

// A code that uniquely identifies the request. That code can
// be put in a response, so that it is clear to which request we
// respond.
function makeRequestCode(x) {
  return "src" + x.src + "_dst" + x.dst + "_seqNumber" + x.seqNumber;
}

// Run a remote script. cb is called with a string as a result,
// that serves as a reference to the request.
// Remember that it might take weeks for the actual response
// to come back to us after having run the script remotely: First,
// our script packet must propagate to the destination, then run there, and then
// 
function runRemoteScript(mailbox, dstMailboxName, type, script, reqCode, cb) {
  if (!(!reqCode || (typeof reqCode == 'string'))) {
    cb(new Error('Bad request code passed to runRemoteScript: ' + reqCode));
  } else {
    if (!(mailbox.sendPacketAndReturn && common.isIdentifier(dstMailboxName)
          && validScriptType(type) && (typeof script == 'string'))) {
      cb(new Error("runRemoteScript: Bad inputs"));
    } else {
      mailbox.sendPacketAndReturn(
        dstMailboxName, common.scriptRequest,
        packScriptRequest(type, script, reqCode),
        function(err, packetData) {
          if (err) {
            cb(err);
          } else if (!packetData) {
            cb(new Error('The sendPacket method of mailbox does not work as expected.'));
          } else {
            cb(null, reqCode || makeRequestCode(packetData));
          }
        });
    }
  }
}

function fileExists(filename, cb) {
  fs.stat(filename, function(err, stat) {
    if (err == null) {
      cb(null, true);
    } else if (err.code == 'ENOENT') {
      cb(null, false);
    } else {
      cb(err);
    }
  });
}

function generateScriptFilename(type, counter, cb) {
  if (!bigint.isBigInt(counter)) {
    counter = bigint.makeFromTime();
  }
  var filename = '/tmp/script' + counter + "." + type;
  fileExists(filename, function(err, p) {
    if (err) {
      cb(err);
    } else {
      if (p) {
        generateScriptFilename(type, bigint.inc(counter), cb);
      } else {
        cb(null, filename);
      }
    }
  });
}

function sendResponse(mailbox, dst, data, cb) {
  mailbox.sendPacket(dst, common.scriptResponse, packScriptResponse(data), cb);
}


function executeAndRespondJS(reqCode, mailbox, script, packet, cb) {
  try {
    var sendTheResponse = function(data) {
      sendResponse(mailbox, packet.src, data, cb);
    }
    var main = eval(script);
    if (!(typeof main == "function")) {
      sendtheResponse({reqCode: reqCode, err: 'The script must return a functionto be evaluated'});
    } else {
      main(function(err, result) {
        if (err) {
          cb(err);
        } else {
          sendTheResponse({reqCode: reqCode, err: err, result: result});
        }
      });
    }
  } catch (e) {
    sendTheResponse({reqCode: reqCode, err: ('' + e)});
  }
}

function executeAndRespondSH(reqCode, mailbox, script, packet, cb) {
  try {
    var sendTheResponse = function(data) {
      sendResponse(mailbox, packet.src, data, cb);
    }
    exec(script,
      function (error, stdout, stderr) {
        sendTheResponse({reqCode: reqCode, err:error, stdout: stdout, stderr: stderr});
      });
  } catch (e) {p
    sendTheResponse({reqCode: reqCode, err: ('' + e)});
  }
}

function executeScriptAndRespond(mailbox, script, packet, type, reqCode, cb) {
  if (!reqCode) {
    reqCode = makeRequestCode(packet);
  }
  if (type == 'js') {
    executeAndRespondJS(reqCode, mailbox, script, packet, cb);
  } else if (type == 'sh') {
    executeAndRespondSH(reqCode, mailbox, script, packet, cb);
  }
}

function handleScriptRequest(mailbox, packet, done, cb) {
  if (packet.label == common.scriptRequest) {
    var req = unpackScriptRequest(packet.data);
    if (validScriptType(req.type)) {
      executeScriptAndRespond(mailbox, req.script, packet, req.type, req.reqCode, function(err) {
        if (err) {
          cb(err);
        } else {
          done();
          cb();
        }
      });
    } else {
      cb(new Error('Received script request of invalid type: ' + req.type));
    }
  }
}


function makeScriptRequestHandler(done) {
  done = done || function() {};
  return function(endPoint, packet) {
    handleScriptRequest(endPoint, packet, done, function(err) {
      if (err) {
        console.log('Failed to handle script request with this error:');
        console.log(err);
      }
    });
  };
}

module.exports.makeScriptRequestHandler = makeScriptRequestHandler;
module.exports.runRemoteScript = runRemoteScript;
module.exports.unpackScriptResponse = unpackScriptResponse;

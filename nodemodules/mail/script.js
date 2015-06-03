var common = require('./common.js');
var msgpack = require('msgpack-js');
var bigint = require('./bigint.js');
var path = require('path');

function validScriptType(type) {
  return type == "sh" || type == "js";
}

function packScriptRequest(type, script) {
  var x = {type:type, script:script};
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
function runRemoteScript(mailbox, dstMailboxName, type, script, cb) {
  if (!(mailbox.sendPacket && common.isIdentifier(dstMailboxName)
        && validScriptType(type) && (typeof script == 'string'))) {
    cb(new Error("runRemoteScript: Bad inputs"));
  } else {
    mailbox.sendPacket(
      dstMailboxName, common.scriptRequest,
      packScriptRequest(type, script), function(err, packetData) {
        console.log('packetData = ' + packetData);
        if (err) {
          cb(err);
        } else if (!packetData) {
          cb(new Error('The sendPacket method of mailbox does not work as expected.'));
        } else {
          cb(null, makeRequestCode(packetData));
        }
      });
  }
}

function generateScriptFilename(type, counter, cb) {
  if (!bigint.isBigInt(counter)) {
    counter = bigint.makeFromTime();
  }
  var filename = '/tmp/script' + counter + "." + type;
  path.exists(filename, function(p) {
    if (p) {
      generateScriptFilename(type, bigint.inc(counter), cb);
    } else {
      cb(null, filename);
    }
  });
}

function sendResponse(mailbox, dst, data, cb) {
  mailbox.sendPacket(dst, common.scriptResponse, packScriptResponse(data), cb);
}


function executeAndRespondJS(reqCode, mailbox, filename, packet, cb) {
  try {
    var main = require(filename);
    if (!(typeof main == "function")) {
      cb(new Error('The script must export a function'));
    } else {
      main(function(err, result) {
        if (err) {
          cb(err);
        } else {
          var data = {reqCode: reqCode, err: err, result: result};
          sendResponse(mailbox, packet.src, data, cb);
        }
      });
    }
  } catch (e) {
    cb(e);
  }
}

function executeAndRespondSH(reqCode, mailbox, filename, packet, cb) {
  try {
    exec(
      'sh ' + filename,
      function (error, stdout, stderr) {
        var data = {reqCode: reqCode, error:error, stdout: stdout, stderr: stderr};
        sendResponse(mailbox, packet.src, data, cb);
      });
  } catch (e) {
    cb(e);
  }
}

function executeScriptAndRespond(mailbox, filename, packet, type, cb) {
  var reqCode = makeRequestCode(packet);
  if (type == 'js') {
    executeAndRespondJS(reqCode, mailbox, filename, packet, cb);
  } else if (type == 'sh') {
    executeAndRespondSH(reqCode, mailbox, filename, packet, cb);
  }
}

function handleScriptRequest(mailbox, packet, done, cb) {
  if (packet.label == common.scriptRequest) {
    var req = unpackScriptRequest(packet.data);
    if (validScriptType(req.type)) {
      generateScriptFilename(req.type, null, function(err, filename) {
        if (err) {
          cb(err);
        } else {
          fs.write(filename, req.script, function(err) {
            if (err) {
              cb(err);
            } else {
              executeScriptAndRespond(mailbox, filename, packet, req.type, function(err) {
                if (err) {
                  cb(err);
                } else {
                  done();
                  cb();
                }
              });
            }
          });
        }
      });
    } else {
      cb(new Error('Received script request of invalid type: ' + req.type));
    }
  }
}


function makeScriptRequestHandler(done) {
  done = done || function() {};
  return function(mailbox, packet, T, cb) {
    cb();
    strongLog('GOT PACKET: ' + packet);
    handleScriptRequest(mailbox, packet, done, function(err) {
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

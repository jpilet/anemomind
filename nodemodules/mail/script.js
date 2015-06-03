var common = require('./common.js');
var msgpack = require('msgpack-js');

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

function packScriptResponse(requestCode, response) {
  var x = {requestCode: requestCode, response: response};
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
function runRemoteScript(mailbox, dstMailboxName, type, script, cb) {
  if (!(mailbox.sendPacket && common.isIdentifier(dstMailboxName)
        && validScriptType(type) && (typeof script == 'function'))) {
    cb(new Error("runRemoteScript: Bad inputs"));
  } else {
    mailbox.sendPacket(
      dstMailboxName, common.scriptRequest,
      packScriptRequest(type, script), function(err, packetData) {
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

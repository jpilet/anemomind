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

function runRemoteScript(mailbox, dstMailboxName, type, script, cb) {
  if (!(mailbox.sendPacket && common.isIdentifier(dstMailboxName)
        && validScriptType(type) && (typeof script == 'function'))) {
    cb(new Error("runRemoteScript: Bad inputs"));
  } else {
    mailbox.sendPacket(dstMailboxName, common.scriptRequest
                       packScriptRequest(type, script));
  }
}

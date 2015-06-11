var script = require('mail/script.js');
var common = require('mail/common.js');
var BoxExec = require('./boxexec.model.js');
var path = require('path');
var mkdirp = require('mkdirp');
var fs = require('fs');

function toStringIfNotNull(x) {
  if (x) {
    return '' + x;
  }
  return null;
}

function makeScriptResponseHandler(cbHandled) {
  cbHandled = cbHandled || function() {};
  return function(mailbox, packet, T, cb) {
    cb();
    if (packet.label == common.scriptResponse) {
      var data = script.unpackScriptResponse(packet.data);

      // Save it to the database
      BoxExec.findByIdAndUpdate(
        data.reqCode, {
          complete: true,
          err: toStringIfNotNull(data.err),
          stdout: toStringIfNotNull(data.stdout),
          stderr: toStringIfNotNull(data.stderr)
        }, cbHandled);
    }
  };
}

module.exports = makeScriptResponseHandler;

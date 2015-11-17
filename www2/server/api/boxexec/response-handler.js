var script = require('endpoint/script.js');
var common = require('endpoint/common.js');
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
  return function(endpoint, packet) {
    if (packet.label == common.scriptResponse) {
      var data = script.unpackScriptResponse(packet.data);

      // Save it to the database
      BoxExec.findByIdAndUpdate(
        data.reqCode, {
          timeReceived: new Date,
          err: toStringIfNotNull(data.err),
          stdout: toStringIfNotNull(data.stdout),
          stderr: toStringIfNotNull(data.stderr)
        }, cbHandled);
    }
  };
}

module.exports = makeScriptResponseHandler;

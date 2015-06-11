var script = require('mail/script.js');
var common = require('mail/common.js');
var BoxExec = require('./boxexec.model.js');
var path = require('path');
var mkdirp = require('mkdirp');
var fs = require('fs');

function makeScriptResponseHandler(cbHandled) {
  cbHandled = cbHandled || function() {};
  return function(mailbox, packet, T, cb) {
    cb();
    if (packet.label == common.scriptResponse) {
      var data = script.unpackScriptResponse(packet.data);

      // Save it to the database
      BoxExec.create({
        reqCode: data.reqCode,
        err: '' + data.err,
        stdout: '' + data.stdout,
        stderr: '' + data.stderr
      }, cbHandled);
    }
  };
}

module.exports = makeScriptResponseHandler;

var script = require('mail/script.js');
var Script = require('./script.model.js');

function makeScriptResponseHandler(cbHandled) {
  return function(mailbox, packet, T, cb) {
    cb();
    var data = script.unpackScriptResponse(packet.data);
    Script.create({
      reqCode: data.reqCode,
      err: '' + data.err,
      stdout: '' + data.stdout,
      stderr: '' + data.stderr
    }, cbHandled);
  };
}

module.exports = makeScriptResponseHandler;

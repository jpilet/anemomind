var script = require('mail/script.js');
var common = require('mail/common.js');
var Script = require('./script.model.js');
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
      Script.create({
        reqCode: data.reqCode,
        err: '' + data.err,
        stdout: '' + data.stdout,
        stderr: '' + data.stderr
      }, cbHandled);

      // ...also save a log file.
      var logdir = '/tmp/scriptlogs/';
      mkdirp(logdir, 0755, function(err) {
        if (err) {
          console.log('Failed to create ' + logdir + ' directory');
        } else {
          var logFilename = path.join(logdir, data.reqCode + '.txt');
          fs.writeFile(logFilename,
            'err:\n' + data.err + '\n\n' +
            'stdout:\n' + data.stdout + '\n\n' +
            'stderr:\n' + data.stderr, function() {
              console.log('Wrote remote script output to ' + logFilename + '.');
            });
        }
      });
    }
  };
}

module.exports = makeScriptResponseHandler;

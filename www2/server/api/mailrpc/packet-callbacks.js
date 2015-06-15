var file = require('mail/file.js');
var path = require('path');
var mkdirp = require('mkdirp');
var fs = require('fs');
var config = require('../../config/environment');
var exec = require('child_process').exec;
var makeScriptResponseHandler = require('../boxexec/response-handler.js');


function makeLogFilenameFromParts(tgtDir, parsedFilename, counter) {
  if (counter == 0) {
    return path.join(tgtDir, parsedFilename.base);
  } else {
    var versionNumber = counter + 1;
    return path.join(tgtDir, parsedFilename.name + "_version" + versionNumber
                     + parsedFilename.ext);
  }
}

function tryToSaveWithName(tgtDir, parsedFilename, counter, data, cb) {
  var filename = makeLogFilenameFromParts(tgtDir, parsedFilename, counter);
  fs.readFile(filename, function(err, loadedData) {
    if (err) { // <-- No such file with that name.
      fs.writeFile(filename, data, cb);
    } else {
      if (loadedData.equals(data)) {
        
        // No point in saving the same data twice with different names
        cb();
        
      } else {
        // We don't want to end up here, but if we do, make sure the data is saved.
        console.log('WARNING (when saving incoming log file): ' +
                    'There is already a file with name ' + filename
                    + ' but different data');
        tryToSaveWithName(tgtDir, parsedFilename, counter + 1, data, cb);
      }
    }
  });
}

function saveLogFile(tgtDir, msg, cb) {
  var data = msg.data;
  var parsed = path.parse(msg.path);
  mkdirp(tgtDir, function(err) {
    if (err) {
      cb(err);
    } else {
      tryToSaveWithName(tgtDir, parsed, 0, data, cb);
    }
  });
}

var pushRunning = false;
function pushLogFilesToProcessingServer() {
  if (pushRunning) {
    return;
  }
  pushRunning = true;
  var args = [
      'rsync',
      '--remove-source-files',
      '-e ssh',
      '-ar',
      '"' + config.uploadDir + '"/*',
      'anemomind@vtiger.anemomind.com:userlogs'
  ];
  var cmd = args.join(' ');
  exec(cmd,
       function(err, stdout, stderr) {
         if (err) {
           console.log('Error while running: ' + cmd);
           console.log(err);
         }
         pushRunning = false;
       });
}

function getTargetDirectory(mailbox) {
  // Is there a better place to put them?
  return path.join(config.uploadDir, "anemologs", mailbox.name);
}  

// Please list below all the callbacks that should be called,
// sequentially, whenever a packet is received
module.exports.add = function(endPoint) {
  endPoint.addPacketHandler(function(endPoint, packet) {
    if (file.isLogFilePacket(packet)) {
      var msg = file.unpackFileMessage(packet.data);
      var tgtDir = getTargetDirectory(endPoint);
      saveLogFile(tgtDir, msg, function(err) {
        if (err) {
          console.log("Error when trying to save incoming log file:");
          console.log(err);
        } else {
          pushLogFilesToProcessingServer();
        }
      });
    }
  });
  endPoint.addPacketHandler(makeScriptResponseHandler());
};

var file = require('endpoint/logfile.js');
var path = require('path');
var mkdirp = require('mkdirp');
var fs = require('fs');
var config = require('../../config/environment');
var makeScriptResponseHandler = require('../boxexec/response-handler.js');
var backup = require('../../components/backup');


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
      fs.writeFile(filename, data, function(err) {
        console.log('Wrote the log file ' + filename);
        cb(err);
      });
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

function getTargetDirectory(endpoint) {
  // Is there a better place to put them?
  return path.join(config.uploadDir, "anemologs", endpoint.name);
}  

// Please list below all the callbacks that should be called,
// sequentially, whenever a packet is received
module.exports.add = function(endpoint) {
  // To handle an incoming log file.
  endpoint.addPacketHandler(function(endpoint, packet) {
    if (file.isLogFilePacket(packet)) {
      var msg = file.unpackFileMessage(packet.data);
      var tgtDir = getTargetDirectory(endpoint);
      saveLogFile(tgtDir, msg, function(err) {
        if (err) {
          console.log("Error when trying to save incoming log file:");
          console.log(err);
        } else {
          backup.pushLogFilesToProcessingServer();
        }
      });
    }
  });

  // To handle the incoming response of running a script
  endpoint.addPacketHandler(makeScriptResponseHandler());
};

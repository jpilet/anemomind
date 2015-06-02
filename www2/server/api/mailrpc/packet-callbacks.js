var file = require('mail/file.js');
var path = require('path');
var mkdirp = require('mkdirp');
var fs = require('fs');
var config = require('../../config/environment');


// http://stackoverflow.com/questions/9080085/node-js-find-home-directory-in-platform-agnostic-way
function getUserHome() {
  return process.env[(process.platform == 'win32') ? 'USERPROFILE' : 'HOME'];
}

function fileExists(filename, cb) {
  fs.readFile(filename, function(err, data) {
    cb(undefined, !err);
  });
}

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
  fileExists(filename, function(err, p) {
    if (err) {
      cb(err);
    } else {
      if (p) {
        console.log('WARNING (when saving incoming log file): ' +
                    'There is already a file with name ' + filename);
        tryToSaveWithName(tgtDir, parsedFilename, counter + 1, data, cb);
      } else {
        fs.writeFile(filename, data, cb);
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

function getTargetDirectory(mailbox) {
  // Is there a better place to put them?
  return path.join(config.uploadDir, "anemologs", mailbox.mailboxName);
}  

// Please list below all the callbacks that should be called,
// sequentially, whenever a packet is received
module.exports.onPacketReceived = 
  function(mailbox, packet, T, cb) {
    cb();
    if (file.isLogFilePacket(packet)) {
      var msg = file.unpackFileMessage(packet.data);
      var tgtDir = getTargetDirectory(mailbox);
      saveLogFile(tgtDir, msg, function(err) {
        if (err) {
          console.log("Error when trying to save incoming log file:");
          console.log(err);
        }
      });
    }
  };

// Please list below all the callbacks that should be called,
// sequentially, whenever an ack packet is received.
module.exports.onAcknowledged = [

    // As for onPacketReceived, maybe we want to do something once
    // we receive an acknowledgment packet for packets previously sent.
    //
    // For instance, if we were transferring a file, we might want to delete that file
    // or something once we know all its pieces has reached the destination.
    function(mailbox, T, data, cb) {
	console.log('Received acknowledgement for packets previously sent: %j', data);
	cb();
    }
];

// File transfer facilities
var common = require('./common.js');
var mb = require('./mail.sqlite.js');
var fs = require('fs');
var assert = require('assert');
var msgpack = require('msgpack');

// fs.writeFile(filename, data, cb)


function isFileMessage(x) {
  if (typeof x == 'object') {
    return x.path && x.info && x.data;
  }
  return false;
}

function packFileData(path, info, cb) {
  fs.readFile(path, function(err, data) {
    if (err) {
      cb(err);
    } else {
      
      // We could just send the buffer,
      // but I think it makes sense to send it together with
      // some context.
      var message = {
	path: path, // The path of the file on the sender file system.
	info: info, // Extra information about the file, free format. E.g. a tag saying that
	            // it is a log file.
	data: data  // The file data.
      };

      assert(isFileMessage(message));
           
      // Call the callback with the packed packet.
      cb(null, msgpack.pack(message));
    }
  });
}

// Send a file
function sendFile(mailbox,   // The local mailbox
		  dst,       // destination mailbox
		  path,      // Path to the file to send
		  info,      // Misc information for the user to choose.
		  cb) {      // Called once the file has been put in the mailbox,
                             //   waiting to be sent.
  if (!(mb.isMailbox(mailbox) && common.isIdentifier(dst))) {
    cb(new Error('Bad input to sendFile'));
  } else {
    packFileData(path, info, function(err, buf) {
      mailbox.sendPacket(dst, common.file, buf, cb);
    });
  }
}

function unpackFile(packet) {
  msgpack.unpack(buf);
}


module.exports.sendFile = sendFile;

// File transfer facilities
var commont = require('./common.js');
var mb = require('./mail.sqlite.js');
var fs = require('fs');


function packFileData(path, info, cb) {
  fs.readFile(path, function(err, data) {
    if (err) {
      cb(err);
    } else {
      // We could just send the buffer,
      // but I think it makes sense to send it together with
      // some context.
      packet = {
	path: path, // The path of the file on the sender file system.
	info: info, // Extra information about the file, free format. E.g. a tag saying that
	            // it is a log file.
	data: data  // The file data.
      };
    }
  });
}

// Send a file from the current mailbox.
function sendFile(mailbox, dst, path, desc, cb) {
  if (!(mb.isMailbox(mailbox))) {
    cb(new Error('Bad input to sendFile'));
  } else {
    packFileData(path, 
  }
}

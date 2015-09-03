// File transfer facilities
var common = require('./common.js');
var mb = require('./endpoint.sqlite.js');
var fs = require('fs');
var assert = require('assert');

/*
  As documented here, 'msgpack' doesn't accurately encode node buffers:
  
  http://stackoverflow.com/questions/13924936/encoding-messagepack-objects-containing-node-js-buffers

  Therefore, we use msgpack-js instead.
  */
  
var msgpack = require('msgpack-js');

// fs.writeFile(filename, data, cb)


function isFileMessage(x) {
  return common.isObjectWithFields(x, ['path', 'info', 'data']);
}

function packFileMessage(path, info, data) {
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
  return msgpack.encode(message);
}

function readAndPackFile(path, info, cb) {
  fs.readFile(path, function(err, data) {
    if (err) {
      cb(err);
    } else {
      cb(null, packFileMessage(path, info, data));
    }
  });
}

// Send a file
function sendFile(endpoint,   // The local endpoint
		  dst,       // destination endpoint
		  path,      // Path to the file to send
		  info,      // Misc information for the user to choose.
                  label,     // The label used for the packet
		  cb) {      // Called once the file has been put in the endpoint,
                             //   waiting to be sent.
  if (!(mb.isEndpoint(endpoint) && common.isIdentifier(dst))) {
    cb(new Error('Bad input to sendFile'));
  } else {
    readAndPackFile(path, info, function(err, buf) {
      endpoint.sendPacket(dst, label, buf, cb);
    });
  }
}

function sendLogFile(endpoint, dst, path, cb) {
  sendFile(endpoint, dst, path,
           makeLogFileInfo(), common.logfile, cb);
}

function unpackFileMessage(buf) {
  return msgpack.decode(buf);
}


///////////////////
// Log files
function isLogFileInfo(info) {
  if (common.isObjectWithFields(info, ['type'])) {
    return info.type == 'logfile';
  }
  return false;
}

// This function might accept more
// arguments in future.
function makeLogFileInfo() {
  return {type: 'logfile'}
}

function isLogFilePacket(pkt) {
  return pkt.label == common.logfile;
}


module.exports.sendLogFile = sendLogFile;
module.exports.sendFile = sendFile;
module.exports.unpackFileMessage = unpackFileMessage;
module.exports.isLogFileInfo = isLogFileInfo;
module.exports.makeLogFileInfo = makeLogFileInfo;
module.exports.readAndPackFile = readAndPackFile;
module.exports.isLogFilePacket = isLogFilePacket;

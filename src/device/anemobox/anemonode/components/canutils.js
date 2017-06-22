function getMessages(cb) {
  var rawcan = require('rawcan');
  var can = rawcan.createSocket('can0');
  can.on('error', function(err) { 
    cb(err);
  });
  can.on('message', function(msg) {
    cb(null, msg);
  });
}

function makeError(msg) {
  return "ERROR " + msg;
}

function getError(x) {
  if (5 < x.length && x.slice(0, 5) == "ERROR") {
    return x.slice(6);
  }
  return null;
}

function isNumber(x) {
  return typeof x == "number";
}

function serializeMessage(msg) {
  if (!(isNumber(msg.ts_sec) && 
  	isNumber(msg.ts_usec) && isNumber(msg.id))) {
    return makeError("Bad message format");;
  }
  if (!(msg.data instanceof Buffer)) {
    return makeError("data is not a buffer");
  }
  var dst = new Buffer(8 + 8 + 8 + msg.data.length);
  dst.writeDoubleBE(msg.ts_sec, 0);
  dst.writeDoubleBE(msg.ts_usec, 8);
  dst.writeDoubleBE(msg.id, 16);
  msg.data.copy(dst, 24);
  return dst.toString('hex');
}

function deserializeMessage(src0) {
  console.log("It is ");
  if (!(typeof src0 == "string")) {
    return {error: "Not a string"};
  }
  var err = getError(src0);
  if (err) {
    return {error: err};
  }
  var src = new Buffer(src0, 'hex');
  if (src.length < 24) {
    return {error: "Too short message"};
  }
  return {
    ts_sec: src.readDoubleBE(0),
    ts_usec: src.readDoubleBE(8),
    id: src.readDoubleBE(16),
    data: src.slice(24)
  };
}

function startCanSource() {
  getMessages(function(err, msg) {
    if (err) {
      process.stdout.write(makeError("Got error in getMessages callback") + "\n");
    } else {
      process.stdout.write(serializeMessage(msg) + "\n");
    }
  });
}

module.exports.serializeMessage = serializeMessage;
module.exports.deserializeMessage = deserializeMessage;
module.exports.getError = getError;
module.exports.startCanSource = startCanSource;
var util = require('util');
var bleno = require('bleno');
var zlib = require('zlib');
 
var BlenoCharacteristic = bleno.Characteristic;

var ANSWER_CODE = 0;

var rpcFuncTable = {};

var mtu = bleno.mtu;

bleno.onMtuChange = function(newMtu) {
  mtu = newMtu;
  console.log('MTU is now: ' + mtu);
}

function RpcCharacteristic() {
  this.pushBuffer = undefined;
  this.bufferToSend = undefined;
  this.sentBytes = 0;
  this.receivingBuffer = undefined;
  this.sendQueue = [ ];
  this.pendingCalls = { };

  RpcCharacteristic.super_.call(this, {
    uuid: 'AFF1E42D-EF91-456F-86FA-87032000CA11',
    properties: ['notify', 'write']
  });
}
util.inherits(RpcCharacteristic, BlenoCharacteristic);

RpcCharacteristic.prototype.call = function(func, args, callback) {
  var packet = {
    callId: Math.round(Math.random() * 65535),
    func: func,
    args: args
  };

  this.pendingCalls[callId] = callback;

  zlib.deflate(JSON.stringify(packet), function(err, buffer) {
    if (err) {
      callback(err, undefined);
    } else {
      this.sendBuffer(buffer);
    }
  });
}

RpcCharacteristic.prototype.handleReceivedBuffer = function(buffer) {
  var rpcble = this;

  zlib.unzip(buffer, function(err, buffer) {
    if (err) {
      console.log('btrpc: ignoring a bad RPC buffer');
      return;
    }
    var packet = JSON.parse(buffer.toString());

    if ('answerId' in packet) {
      // We received an answer. Let's check we actually asked for it.
      if (packet.answerId in rpcble.pendingCalls) {
        rpcble.pendingCalls[packet.answerId]('answer' in packet ? packet.answer : undefined);
        delete rpcble.pendingCalls[pcaket.answerId];
      } else {
        console.log('answerId does not match the know callIds: ' + packet.answerId);
      }
    } else if ('func' in packet && packet.func in rpcFuncTable
               && 'callId' in packet) {
      // We received a valid call.
      var rpcFunc = rpcFuncTable[packet.func];

      rpcFunc(('args' in packet ? packet.args : undefined), function(answer) {
        var reply = {
          answerId: packet.callId,
          answer: answer
        };
        zlib.deflate(JSON.stringify(reply), function(err, buffer) {
          if (err) {
            throw(err);
          } else {
            console.log('sending ' + buffer.length + ' bytes');
            rpcble.sendBuffer(buffer);
          }
        });
      });
    } else {
      console.log('malformed btrpc call: ' + buffer.toString());
    }
  });
};

RpcCharacteristic.prototype.sendBuffer = function(buffer, callback) {
  this.sendQueue.push({data: buffer, cb: callback});
  this.sendNextBuffer();
}

RpcCharacteristic.prototype.sendNextBuffer = function() {
  if (this.bufferToSend) {
    // A buffer is already being sent.
    return;
  }

  if (this.sendQueue.length == 0) {
    // nothing to send
    return;
  }

  if (this.pushBuffer == undefined) {
    // nobody is connected.
    return;
  }

  this.bufferToSend = this.sendQueue.shift();
  this.sentBytes = 0;
  this.sendNextChunk();

};

RpcCharacteristic.prototype.sendNextChunk = function() {
  var buffer = this.bufferToSend.data;
  var chunkLimit = this.sentBytes + mtu;
  var sendLimit = Math.min(chunkLimit, buffer.length);

  this.pushBuffer(buffer.slice(this.sentBytes, sendLimit));
  this.sendingEOM = false;

  this.sentBytes = sendLimit;
};

RpcCharacteristic.prototype.onSubscribe =
    function(maxValueSize, updateValueCallback) {
  this.pushBuffer = updateValueCallback;
  this.sendNextBuffer();
};

RpcCharacteristic.prototype.onUnsubscribe = function() {
  if (this.bufferToSend) {
    this.unshift(this.bufferToSend);
    this.bufferToSend = undefined;
  }
  this.pushBuffer = undefined;
};

RpcCharacteristic.prototype.onNotify = function() {
  // Called when the phone has been notified of a value change.
 
  if (this.sendingEOM) {
    this.sendingEOM = false;
    // We finished to send this packet. Notify the caller.
    if (typeof(this.bufferToSend.cb) == 'function') {
      this.bufferToSend.cb();
    }
    this.bufferToSend = undefined;
    this.sendNextBuffer();
  } else if (this.bufferToSend.data.length <= this.sentBytes) {
    // everything is sent. Send EOM.
    this.pushBuffer(new Buffer("==EOM=="));
    this.sendingEOM = true;
  } else {
    // We are in the middle of a transfer, send the next chunk.
    this.sendNextChunk();
  }
};

RpcCharacteristic.prototype.onWriteRequest = function(data, offset, withoutResponse, callback) {

  if (data.toString('ascii') == "==EOM==") {
    this.handleReceivedBuffer(this.receivingBuffer);
    this.receivingBuffer = undefined;
  } else {
    if (!this.receivingBuffer) {
      // We start a new file.
      this.receivingBuffer = data;
    } else {
      this.receivingBuffer = Buffer.concat([this.receivingBuffer, data]);
    }
  }
  callback(this.RESULT_SUCCESS);
};


//module.exports.rpc = new RpcCharacteristic();

module.exports.pushCharacteristics = function(array) {
  var rpcChar = new RpcCharacteristic();
  array.push(rpcChar);
}
module.exports.rpcFuncTable = rpcFuncTable;


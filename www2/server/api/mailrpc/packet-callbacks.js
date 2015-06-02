var file = require('mail/file.js');

var logRoot = 

// Please list below all the callbacks that should be called,
// sequentially, whenever a packet is received
module.exports.onPacketReceived = 
  function(mailbox, T, packet, cb) {
    cb();
    if (file.isLogFilePacket(packet)) {
      var msg = file.unpackFileMessage(packet.data);
      
    }
  }


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

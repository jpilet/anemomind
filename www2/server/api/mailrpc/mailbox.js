var mb = require('mail/mail2.sqlite.js');
var packetCallbacks = require('./packet-callbacks.js');
var common = require('mail/common.js');
var naming = require('mail/naming.js');
var mkdirp = require('mkdirp');
var config = require('../../config/environment');
var path = require('path');
var schema = require('mail/endpoint-schema.js');

/*

  On the server, you typically want to do this:

  * send packet(s): Use the exports.sendPacket(s) to do that.

  * Receive packets: Put your handler inside the onPacketReceived array in packet-callbacks.js
  
  * In some cases, you might want to do something once you know that a packet
  that you sent reached its destination. In that case, put your handler
  in the onAcknowledged array, in packet-callbacks.js.

*/

// Conveniency function for sending a packet:
// It will open a mailbox, send packets from
// that mailbox, and close the mailbox.
function sendPackets(src, dst, label, dataArray, cb) {
  openMailbox(
    src,
    function(err, mailbox) {
      if (err) {
	cb(err);
      } else {
	mailbox.sendPackets(
	  dst,
	  label,
	  dataArray,
	  function (err) {
	    if (err) {
	      cb(err);
	    } else {
	      mailbox.close(
		cb
	      );			    
	    }
	  }
	);
      }
    }
  );
}

function sendPacket(src, dst, label, data, cb) {
  sendPackets(src, dst, label, [data], cb);
}

function openMailbox(mailboxName, cb) {
  if (!common.isValidMailboxName(mailboxName)) {
    cb(new Error('Invalid mailbox name: ' + mailboxName));
  } else {
    mkdirp(config.mailboxDir, 0755, function(err) {
      if (err) {
        cb(err);
      } else {
        var filename = path.join(
          config.mailboxDir,
          naming.makeDBFilename(mailboxName));
        mb.tryMakeEndPoint(
          filename, mailboxName,
          function(err, mailbox) {
	    if (err) {
	      cb(err);
	    } else {
              schema.makeVerbose(mailbox);
	      packetCallbacks.add(mailbox);
	      cb(err, mailbox);
	    }
          });
      }
    });
  }
}

exports.openMailbox = openMailbox;
exports.sendPackets = sendPackets;
exports.sendPacket = sendPacket;

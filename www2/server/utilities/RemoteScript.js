var naming = require('mail/naming.js');
var mb = require('mail/mail.sqlite.js');
var common = require('mail/common.js');
var script = require('mail/script.js');
var appRoot = require('app-root-path');

/*

Todo: Obtain the boxId given a boatId.
  
*/  

function openMailbox(mailboxName, cb) {
  if (!common.isValidMailboxName(mailboxName)) {
    cb(new Error('Invalid mailbox name: ' + mailboxName));
  } else {
    var filename = appRoot + '/' + mailboxName + '.mailsqlite.db';
    console.log('filename    = ' + filename);
    console.log('mailboxName = ' + mailboxName);
    mb.tryMakeMailbox(
      filename, mailboxName,
      function(err, mailbox) {
	if (err) {
	  cb(err);
	} else {
	  cb(err, mailbox);
	}
      }
    );
  }
}

function executeScript(boatId, boxId, type, script, cb) {
  for (var i = 0; i < 30; i++) {
    console.log('CB');
    console.log(cb);
  }
  openMailbox(
    naming.makeMailboxNameFromBoatId(boatId),
    function(err, mailbox) {
      var dstName = naming.makeMailboxNameFromBoxId(boxId);
      console.log('THE MAILBOX IS ');
      console.log(mailbox);
      console.log('type   = '+ type);
      console.log('dst    = '+ dstName);
      console.log('script = '+ script);
      console.log('cb     = '+ cb);
      
      script.runRemoteScript(
        mailbox, dstName,
        type, script, cb);
    });
}

function makeOptionalCB(x) {
  return x || function() {};
}

function executeJS(boatId, boxId, script, cbOptional) {
  executeScript(boatId, boxId, 'js', script, makeOptionalCB(cbOptional));
}

function executeSH(boatId, boxId, script, cbOptional) {
  executeScript(boatId, boxId, 'sh', script, makeOptionalCB(cbOptional));
}

module.exports.executeJS = executeJS;
module.exports.executeSH = executeSH;

executeJS('a', 'b', 'rulle');


var path = require('path');
var naming = require('mail/naming.js');
var script = require('mail/script.js');
var mongoose = require('mongoose');
var exec = require('child_process').exec;
var Boat = require('../server/api/boat/boat.model.js');
var Q = require('q');
var mb = require('mail/mail2.sqlite.js');
var fs = require('fs');
var BoxExec = require('../server/api/boxexec/boxexec.model.js');

// Ensure NODE_ENV is defined.
process.env.NODE_ENV = process.env.NODE_ENV || 'development';
var env = require('../server/config/environment');

var connectionDeferred = Q.defer();
function openMongoConnection(cb) {
  mongoose.connect(env.mongo.uri, env.mongo.options);
  mongoose.connection.on('open', function(ref) {
    connectionDeferred.resolve(ref);
    cb(ref);
  });
}

function init() {
  openMongoConnection(function(err) {
    if (err) {
      console.log('Failed to initialize RemoteScriptCommon.js module:');
      console.log(err);
    }
  });
}

function withMongoConnection(cbOperation) {
  connectionDeferred.promise.then(function(value) {
    cbOperation(value);
  });
}


function extractBoatIdFromFilename(filename) {
  var mailboxName = naming.getMailboxNameFromFilename(filename);
  var parsed = naming.parseMailboxName(mailboxName);
  return parsed.id;
}

function getBoxIdFromBoatId(boatId, cb) {
  Boat.findById(boatId, function(err, results) {
    if (err) {
      cb(err);
    } else {
      if (results) {
        if (results.anemobox) {
          cb(null, results.anemobox);
        } else {
          cb(new Error('An anemobox has not been assigned to boat with id ' + boatId));
        }
      } else {
        cb(new Error('No document in db for boat with id ' + boatId));
      }
    }
  });
}

function getBoxIdFromFilename(filename, cb) {
  var boatId = extractBoatIdFromFilename(filename);
  getBoxIdFromBoatId(boatId, cb);
}

function sendScriptToBox(filename, scriptType, scriptData, cb_) {
  var globalMailbox = null;
  
  var cb = function(err, data) {
    if (globalMailbox) {
      globalMailbox.close(function(err2) {
        cb_(err || err2, data);
      });
    } else {
      cb_(err, data);
    }
  };
  
  mb.tryMakeEndPointFromFilename(filename, function(err, mailbox) {
    if (err) {
      cb(err);
    } else {
      globalMailbox = mailbox;
      var boatId = extractBoatIdFromFilename(filename);
      getBoxIdFromBoatId(boatId, function(err, boxId) {
        if (err) {
          cb(err);
        } else {
          dst = naming.makeMailboxNameFromBoxId(boxId);
          BoxExec.create({
            timeSent: new Date,
            boatId: boatId,
            boxId: boxId,
            type: scriptType,
            script: scriptData
          }, function(err, boxexec) {
            if (err) {
              cb(err);
            } else {
              script.runRemoteScript(
                mailbox, dst,
                scriptType, scriptData, '' + boxexec._id, cb);
            }
          });
        }
      });
    }
  });
}

function sendScriptFileToBox(dbFilename, scriptFilename, cb) {
  try {
    var parsed = path.parse(scriptFilename);
    var scriptType = parsed.ext.substring(1);
    fs.readFile(scriptFilename, 'utf8', function(err, scriptData) {
      if (err) {
        cb(err);
      } else {
        sendScriptToBox(dbFilename, scriptType, scriptData, cb);
      }
    });
  } catch (e) {
    cb(e);
  }
}

module.exports.extractBoatIdFromFilename = extractBoatIdFromFilename;
module.exports.withMongoConnection = withMongoConnection;
module.exports.getBoxIdFromBoatId = getBoxIdFromBoatId;
module.exports.sendScriptToBox = sendScriptToBox;
module.exports.sendScriptFileToBox = sendScriptFileToBox;
module.exports.init = init;

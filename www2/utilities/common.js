
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
var files = require('mail/files.js');
var assert = require('assert');

// Ensure NODE_ENV is defined.
process.env.NODE_ENV = process.env.NODE_ENV || 'development';
var env = require('../server/config/environment');

function init() {
  mongoose.connect(env.mongo.uri, env.mongo.options);
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

function makeBoatDBFilename(boatId) {
  return path.join(
    env.mailboxDir,
    naming.makeDBFilenameFromBoatId(boatId))
}

function makeBoatEndPoint(boatId, cb) {
  mb.tryMakeEndPoint(
    makeBoatDBFilename(boatId),
    naming.makeMailboxNameFromBoatId(boatId), cb);
}

function withBoatEndPoint(boatId, cbOperation, done) {
  makeBoatEndPoint(boatId, function(err, ep) {
    if (err) {
      done(err);
    } else {
      mb.withEP(ep, cbOperation, done);
    }
  });
}


function sendScriptToBox(boatId, scriptType, scriptData, cb) {
  var code = null;
  withBoatEndPoint(boatId, function(mailbox, cb) {
    assert(typeof cb == 'function');
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
            code = boxexec._id;
            script.runRemoteScript(
              mailbox, dst,
              scriptType, scriptData, '' + boxexec._id, cb);
          }
        });
      }
    });
  }, function(err) {
    if (err) {
      cb(err);
    } else {
      cb(null, code);
    }
  });
}

function sendScriptFileToBox(boatId, scriptFilename, cb) {
  try {
    var parsed = path.parse(scriptFilename);
    var scriptType = parsed.ext.substring(1);
    fs.readFile(scriptFilename, 'utf8', function(err, scriptData) {
      if (err) {
        cb(err);
      } else {
        sendScriptToBox(boatId, scriptType, scriptData, cb);
      }
    });
  } catch (e) {
    cb(e);
  }
}

function makeDstFilename(srcFilename, dstFilename) {
  if (dstFilename) {
    return dstFilename;
  } else {
    var p = path.parse(srcFilename);
    return p.base;
  }
}

function sendBoatData(boatId, srcFilename, dstFilename, cb) {
  withBoatEndPoint(boatId, function(ep, cb) {
    Q.nfcall(getBoxIdFromBoatId, boatId)
      .then(function(boxId) {
        var dstName = naming.makeMailboxNameFromBoxId(boxId);
        return files.sendFiles(
          ep,
          dstName, [{
            src: srcFilename,
            dst: makeDstFilename(srcFilename, dstFilename)
          }]);
      }).nodeify(cb);
  }, cb);
}

module.exports.extractBoatIdFromFilename = extractBoatIdFromFilename;
module.exports.getBoxIdFromBoatId = getBoxIdFromBoatId;
module.exports.sendScriptToBox = sendScriptToBox;
module.exports.sendScriptFileToBox = sendScriptFileToBox;
module.exports.init = init;
module.exports.makeBoatDBFilename = makeBoatDBFilename;
module.exports.sendBoatData = sendBoatData;

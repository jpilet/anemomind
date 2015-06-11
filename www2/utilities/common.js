var path = require('path');
var naming = require('mail/naming.js');
var script = require('mail/script.js');
var mongoose = require('mongoose');
var exec = require('child_process').exec;
var Boat = require('../server/api/boat/boat.model.js');
var Q = require('q');
var mb = require('mail/mail.sqlite.js');

var dev = true;
var mongoOptions = {db: {safe: true}};
var mongoUri = (dev?
                'mongodb://localhost/anemomind-dev' :
                'mongodb://localhost/anemomind');


var connectionDeferred = Q.defer();
function openMongoConnection(cb) {
  mongoose.connect(mongoUri, mongoOptions);
  mongoose.connection.on('open', function(ref) {
    connectionDeferred.resolve(ref);
    cb(ref);
  });
}

// Open a connection when this module
// is loaded.
openMongoConnection(function() {});

function withMongoConnection(cbOperation) {
  connectionDeferred.promise.then(function(value) {
    cbOperation(value);
  });
}


function mongoDemo() {
  openMongoConnection(function (ref) {
    console.log('Connected to mongo server.');
    //trying to get collection names
    mongoose.connection.db.collectionNames(function (err, names) {
      console.log(names); // [{ name: 'dbname.myCollection' }]
      module.exports.Collection = names;
    });

    // Find a particular boat
    Boat.findById("552e25e0e412da9baacacffc", function(err, results) {
      console.log('Boats');
      console.log(results);
    });
  });
}
//mongoDemo();
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
      if (typeof results == 'object') {
        cb(null, results.anemobox);
      } else {
        cb(new Error('Cannot extract anemobox id from this data: "' + results + '"'));
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
  
  mb.tryMakeMailboxFromFilename(filename, function(err, mailbox) {
    if (err) {
      cb(err);
    } else {
      globalMailbox = mailbox;
      getBoxIdFromFilename(filename, function(err, boxId) {
        if (err) {
          cb(err);
        } else {
          dst = naming.makeMailboxNameFromBoxId(boxId);
          script.runRemoteScript(
            mailbox, dst,
            scriptType, scriptData, cb);
        }
      });
    }
  });
}

module.exports.extractBoatIdFromFilename = extractBoatIdFromFilename;
module.exports.withMongoConnection = withMongoConnection;
module.exports.getBoxIdFromBoatId = getBoxIdFromBoatId;
module.exports.sendScriptToBox = sendScriptToBox;

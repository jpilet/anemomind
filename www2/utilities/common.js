var path = require('path');
var naming = require('mail/naming.js');
var mongoose = require('mongoose');
var exec = require('child_process').exec;
var Boat = require('../server/api/boat/boat.model.js');

var dev = true;
var mongoOptions = {db: {safe: true}};
var mongoUri = (dev?
                'mongodb://localhost/anemomind-dev' :
                'mongodb://localhost/anemomind');

function openMongoConnection(cb) {
  mongoose.connect(mongoUri, mongoOptions);
  mongoose.connection.on('open', cb);
}

function withMongoConnection(cbOperation, cbDone) {
  openMongoConnection(function(ref) {
    cbOperation(ref, function(err) {
      mongoose.connection.close();
      cbDone(err);
    });
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
  console.log('filename: ' + filename);
  console.log('mailboxName: ' + mailboxName);
  var parsed = naming.parseMailboxName(mailboxName);
  return parsed.id;
}

function getBoxIdFromBoatId(boatId, cb) {
  openMongoConnection(function (ref) {
    Boat.findById(boatId, function(err, results) {
      mongoose.connection.close();
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
  });
}

module.exports.extractBoatIdFromFilename = extractBoatIdFromFilename;
module.exports.withMongoConnection = withMongoConnection;

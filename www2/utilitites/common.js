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

mongoose.connect(mongoUri, mongoOptions);

// List all tables
mongoose.connection.on('open', function (ref) {
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


// cmd = "mongo --quiet anemomind-dev --eval \"print(db.boats.find({})[0]._id + '')\")";
// exec(cmd, function(err, stdout, stderr) {
//   console.log('err');
//   console.log(err);
//   console.log('stdout');
//   console.log(stdout);
//   console.log('stderr');
//   console.log(stderr);
// });


// Common functions used by the remote script and other utilities

function extractMailboxNameFromFilename(filename) {
  if (typeof filename == 'string') {
    var parsed = path.parse(filename);
    var index = parsed.name.indexOf(".");
    return (index > 0? parsed.name.substring(0, index) : parsed.name);
  }
  return null;
}

function extractBoatIdFromFilename(filename) {
  var mailboxName = extractMailboxNameFromFilename(filename);
  var parsed = naming.parseMailboxName(mailboxName);
  return parsed.id;
}

function getBoxIdFromBoatId(boatId, cb) {
  
}

module.exports.extractMailboxNameFromFilename = extractMailboxNameFromFilename;
module.exports.extractBoatIdFromFilename = extractBoatIdFromFilename;

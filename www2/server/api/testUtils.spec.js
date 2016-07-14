var Q = require('q');
var User = require('./user/user.model');
var Boat = require('./boat/boat.model');
var Event = require('./event/event.model');

var toClean = [];

function addTestBoat(boat) {
  return Q.Promise(function(resolve, reject) {
    var testBoat = new Boat(boat);
    testBoat.save(function(err, boat) {
      if (err) {
        reject(err);
      } else {
        toClean.push(function() { Boat.remove({_id: boat._id}).exec(); });
        resolve(boat);
      }
    });
  });
}

function addTestUser(name) {
  return Q.Promise(function(resolve, reject) {
    var testUser = new User({
        "provider" : "local",
        "name" : name,
        "email" : name + "@test.anemomind.com",
        "hashedPassword" : "bj0zHvlC/YIzEFOU7nKwr+OHEzSzfdFA9PMmsPGnWITGHp1zlL+29oa049o6FvuR2ofd8wOx2nBc5e2n2FIIsg==",
        "salt" : "bGwuseqg/L/do6vLH2sPVA==",
        "role" : "user"
    });
    testUser.save(function(err, user) {
      if (err) {
        reject(err);
      } else {
        toClean.push(function() { User.remove({_id: user._id}).exec(); });
        resolve(user);
      }
    });
  });
}

function cleanup() {
  for (var i in toClean) {
    toClean[i]();
  }
}

module.exports.addTestBoat = addTestBoat;
module.exports.addTestUser = addTestUser;
module.exports.cleanup = cleanup;

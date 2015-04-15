'use strict';

var _ = require('lodash');
var Boat = require('./boat.model');
var User = require('../user/user.model');
var mongoose = require('mongoose');
var Q = require('q');

module.exports.userCanRead = function(user, boat) {
  if (!user || !user.id || !boat) {
    return false;
  }
  var userId = mongoose.Types.ObjectId(user.id);

  return (boat.admins && (_.findIndex(boat.admins, userId) >= 0))
    || (boat.readers && (_.findIndex(boat.readers, userId) >= 0));
}

module.exports.userCanWrite = function(user, boat) {
  if (!user || !user.id || !boat || !boat.admins) {
    return false;
  }
  var userId = mongoose.Types.ObjectId(user.id);
  return _.findIndex(boat.admins, userId) >= 0;
}

function userCanAccessBoatId(checkAccess, userid, boatid) {
  // TODO: Cache results to avoid hitting the database too often.
  return Q.Promise(function(resolve, reject) {
    Boat.findById(boatid, function (err, boat) {
      if(err) { return reject(err); }
      if (checkAccess({id: userid}, boat)) {
        resolve();
      }
      reject();
    });
  });
}

module.exports.readableBoats = function(userid) {
  return Q.Promise(function(resolve, reject) {
    var user = mongoose.Types.ObjectId(userid);
    var query = {$or: [{admins: { $in: [user] } }, {readers: { $in: [user]}}]};

    Boat.find(query, function (err, boats) {
      if(err) { reject(err); }
      resolve(boats);
    });
  });
};

module.exports.userCanReadBoatId = function(userid, boatid) {
  return userCanAccessBoatId(module.exports.userCanRead, userid, boatid);
}

module.exports.userCanWriteBoatId = function(userid, boatid) {
  return userCanAccessBoatId(module.exports.userCanWrite, userid, boatid);
}

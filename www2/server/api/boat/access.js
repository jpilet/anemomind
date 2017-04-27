'use strict';

var _ = require('lodash');
var Boat = require('./boat.model');
var User = require('../user/user.model');
var mongoose = require('mongoose');
var Q = require('q');

module.exports.userCanRead = function(user, boat) {
  if (!boat) {
    return false;
  }
  if (boat.publicAccess === true) {
    return true;
  }

  if (!user || !user.id || !boat) {
    return false;
  }

  if (user.role == 'admin') {
    return true;
  }

  var userId = mongoose.Types.ObjectId(user.id);

  return (boat.admins && (_.findIndex(boat.admins, userId) >= 0))
    || (boat.readers && (_.findIndex(boat.readers, userId) >= 0));
}

module.exports.userCanWrite = function(user, boat) {
  if (!user || !user.id || !boat || !boat.admins) {
    return false;
  }

  if (user.role == 'admin') {
    return true;
  }

  var userId = mongoose.Types.ObjectId(user.id);
  return _.findIndex(boat.admins, userId) >= 0;
}

function userCanAccessBoatId(checkAccess, user, boatid) {
  // TODO: Cache results to avoid hitting the database too often.
  return Q.Promise(function(resolve, reject) {
    Boat.findById(boatid, function (err, boat) {
      if(err) { return reject(err); }
      if (checkAccess(user, boat)) {
        resolve();
      }
      reject();
    });
  });
}

module.exports.readableBoats = function(req) {
  return Q.Promise(function(resolve, reject) {
    var query;
    if (req.user && req.user.role == 'admin') {
      query = {};
    } else if (req.user) {
      var user = mongoose.Types.ObjectId(req.user.id);
      query = {
        $or: [
          {admins: { $in: [user] } },
          {readers: { $in: [user]}}
        ]};

      // By default, we do not list public boats.
      // Otherwise, the app get the boat and tries to synchronize with it.
      // this is a workaround to avoid the bug in the app.
      if (req.query.public) {
        // In the future, we probably want to have a list of followed boats
        // instead of including all public access boats,
        // or limit the number of public boats returned.
        query.$or.push({publicAccess: true});
      }
    } else {
      query = { publicAccess: true };
    }

    Boat.findWithPhotosAndComments(query, function (err, boats) {
      if(err) { reject(err); }
      resolve(boats);
    });
  });
};

module.exports.userCanReadBoatId = function(user, boatid) {
  return userCanAccessBoatId(module.exports.userCanRead, user, boatid);
}

module.exports.userCanWriteBoatId = function(user, boatid) {
  return userCanAccessBoatId(module.exports.userCanWrite, user, boatid);
}

var checkAccess = function(checkFunc, req, res, next) {
  var boat = req.params.boatId || req.params.boat;
  if (!boat) {
    return res.sendStatus(400);
  }

  checkFunc(req.user || {}, boat)
    .then(next)
    .catch(function() {
      return res.sendStatus(req.user ? 403 : 401);
    });
};

exports.boatWriteAccess = function(req, res, next) {
  return checkAccess(module.exports.userCanWriteBoatId, req, res, next);
};

exports.boatReadAccess = function(req, res, next) {
  return checkAccess(module.exports.userCanReadBoatId, req, res, next);
};

exports.boatReadAccessOrRedirect = function(req, res, next) {
  var boat = req.params.boatId || req.params.boat;
  if (!boat) {
    return res.sendStatus(400);
  }

  module.exports.userCanReadBoatId((req.user ? req.user.id : undefined), boat)
    .then(next)
    .catch(function() {
      return res.redirect('/login?d=' + encodeURIComponent(req.originalUrl));
    });
};

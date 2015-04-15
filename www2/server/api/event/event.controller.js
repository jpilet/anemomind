'use strict';

var Q = require('q');
var _ = require('lodash');
var Event = require('./event.model');
var mongoose = require('mongoose');

var boatAccess = require('../boat/access.js');

var canRead = function(req, event) {
  // A user can access a note if he/she is the author
  if (req.user && req.user.id && event.author == req.user.id) {
    return Q.Promise(function(resolve, reject) {
      resolve(req.user.id, event);
    });
  }

  // Otherwise, the user needs write access to the boat the note is
  // attached to.
  return boatAccess.userCanReadBoatId(req.user.id, event.boat);
}

var canWrite = function(req, event) {
  return boatAccess.userCanWriteBoatId(req.user.id, event.boat);
}

// Get the latest readable events
exports.index = function(req, res) {
  try {
  if (!req.user) { return res.send(401); }
  boatAccess.readableBoats(req.user.id)
  .then(function(boats) {
    if (boats.length == 0) {
      return res.json(200, []);
    }
    var query = { boat: { $in : _.map(boats, '_id') } }
    Event.find(query, function (err, events) {
      if(err) { return handleError(res, err); }
      return res.json(200, events);
    });
  })
  .catch(function(err) { res.send(403); });
  } catch(err) {
    console.warn(err);
    console.warn(err.stack);
    res.send(500);
  }
};

// Get a single event
exports.show = function(req, res) {
  Event.findById(req.params.id, function (err, event) {
    if(err) { return handleError(res, err); }
    if(!event) { return res.send(404); }

    canRead(req, event)
    .then(function() { res.json(event); })
    .catch(function(err) { res.send(403); });
  });
};

// Creates a new event in the DB.
exports.create = function(req, res) {
  try {
  if (!req.user) {
    return res.send(401);
  }
  if (!req.body.boat) {
    return res.send(400);
  }
  var checkId = new RegExp("^[0-9a-fA-F]{24}$");

  if (!checkId.test(req.body.boat)) {
    return res.send(400);
  }

  var user = mongoose.Types.ObjectId(req.user.id);
  var event = req.body;
  event.author = user;
  event.boat = mongoose.Types.ObjectId(req.body.boat);

  canWrite(req, event)
    .then(function() {
      // User is allowed to add a note for this boat.

      Event.create(event, function(err, event) {
                   if(err) { return handleError(res, err); }
                   return res.json(201, event);
                   });
    })
    .catch(function(err) {
      res.send(403);
    });
  } catch(err) {
    console.warn(err);
    console.warn(err.stack);
    res.send(500);
  }
};

exports.remove = function(req, res) {
  if (!req.user) {
    return res.send(401);
  }
  if (!req.body.boat) {
    return res.send(400);
  }
  boatAccess.userCanWriteBoatId(req.user.id, req.body.boat)
    .then(function() {
      // User is allowed to add a note for this boat.
      var user = mongoose.Types.ObjectId(req.user.id);
      var event = req.body;
      event.author = user;
      event.boat = mongoose.Types.ObjectId(req.body.boat);

      Event.create(event, function(err, event) {
                   if(err) { return handleError(res, err); }
                   return res.json(201, event);
                   });
    })
    .catch(function(err) { res.send(403); });
};

function handleError(res, err) {
  console.log('error: ' + err);
  return res.send(500, err);
}

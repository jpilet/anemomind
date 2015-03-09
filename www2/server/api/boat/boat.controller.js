'use strict';

var _ = require('lodash');
var Boat = require('./boat.model');
var mongoose = require('mongoose');

var userCanRead = function(user, boat) {
  if (!user.id || !boat) {
    return false;
  }
  return _.findIndex(boat.admins, user.id)
    || _.findIndex(boat.readers, user.id);
}
var userCanWrite = function(user, boat) {
  if (!user.id || !boat || !boat.admins) {
    return false;
  }
  return _.findIndex(boat.admins, user.id);
}

// Get list of boats
exports.index = function(req, res) {
  if (!req.user) { return res.send(403); }
  var user = mongoose.Types.ObjectId(req.user.id);
  var query = {$or: [{admins: { $in: [user] } }, {readers: { $in: [user]}}]};

  Boat.find(query, function (err, boats) {
    if(err) { return handleError(res, err); }
    return res.json(200, boats);
  });
};

// Get a single boat
exports.show = function(req, res) {
  Boat.findById(req.params.id, function (err, boat) {
    if(err) { return handleError(res, err); }
    if (!userCanRead(req.user, boat)) { return res.send(403); }
    if(!boat) { return res.send(404); }
    return res.json(boat);
  });
};

// Creates a new boat in the DB.
exports.create = function(req, res) {
  if (!req.user) { return res.send(403); }
  var user = mongoose.Types.ObjectId(req.user.id);
  var boat = req.body;

  // Make sure the user who creates a boat has
  // administrative rights.
  if (!userCanWrite(req.user, boat)) {
    if (boat.admins && boat.admins instanceof Array) {
      boat.admins.push(user);
    } else {
      boat.admins = [ user ];
    }
  }

  Boat.create(boat, function(err, boat) {
    if(err) { return handleError(res, err); }
    return res.json(201, boat);
  });
};

// Updates an existing boat in the DB.
exports.update = function(req, res) {
  if(req.body._id) { delete req.body._id; }
  Boat.findById(req.params.id, function (err, boat) {
    if (err) { return handleError(res, err); }
    if(!boat) { return res.send(404); }
    if (!userCanWrite(req.user, boat)) { return res.send(403); }
    var updated = _.merge(boat, req.body);
    updated.save(function (err) {
      if (err) { return handleError(res, err); }
      return res.json(200, boat);
    });
  });
};

// Deletes a boat from the DB.
exports.destroy = function(req, res) {
  Boat.findById(req.params.id, function (err, boat) {
    if(err) { return handleError(res, err); }
    if(!boat) { return res.send(404); }
    if (!userCanWrite(req.user, boat)) { return res.send(403); }
    boat.remove(function(err) {
      if(err) { return handleError(res, err); }
      return res.send(204);
    });
  });
};

function handleError(res, err) {
  return res.send(500, err);
}

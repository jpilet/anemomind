'use strict'

var Timeset = require('./timeset.model');
var mongoose = require('mongoose');
var boatAccess = require('../boat/access.js');
var _ = require('lodash');

// Delete
module.exports.deleteTimeset = function(req, res, next) {
  var search = {
    _id: mongoose.Types.ObjectId(req.params.timesetId),
    boat: req.params.boatId + ''
  };
  Timeset.remove(search, function(err) {
    if (err) {
      res.sendStatus(422);
    } else {
      res.sendStatus(200);
    }
  });
};

// Get a full list
module.exports.getTimesetsForBoat = function(req, res, next) {
  var boatId = req.params.boatId;
  var search = { boat: boatId + ''};
  Timeset.find(search, function(err, timesets) {
    if (err) {
      res.status(422).end();
    } else {
      res.status(200).json(timesets);
    }
  });
};

module.exports.addTimeset = function(req, res) {
  var timeset = req.body;
  if (timeset.boat != req.params.boatId) {
    return res.sendStatus(400);
  }
  if (!timeset.lower && !timeset.upper) {
    return res.sendStatus(400);
  }
  if (!!timeset.lower && !!timeset.upper && !(timeset.lower <= timeset.upper)) {
    return res.sendStatus(400);
  }
  Timeset.create(timeset, function(err, createdTimeset) {
    if (err) {
      res.sendStatus(500);
    } else {
      res.status(200).json(createdTimeset);
    }
  });
}


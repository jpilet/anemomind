'use strict'

var Timeset = require('./timeset.model');
var mongoose = require('mongoose');
var boatAccess = require('../boat/access.js');
var _ = require('lodash');

// Delete
module.exports.deleteTimeset = function(req, res, next) {
  boatAccess.userCanWriteBoatId(req.user, req.params.boatId)
    .then(function() {
      var search = {
        _id: req.params.timesetId,
        boat: req.params.boatId
      };
      Timeset.remove(search, function(err) {
        if (err) {
          res.sendStatus(422);
        } else {
          res.sendStatus(200);
        }
      });
    }).catch(function(err) { 
      res.sendStatus(403); 
    });
};

// Get a full list
module.exports.getTimesetsForBoat = function(req, res, next) {
  var boatId = req.params.boatId;
  var search = { boat: boatId };
  Timeset.find(search, function(err, timesets) {
    if (err) {
      res.status(422).end();
    } else {
      res.status(200).json(timesets);
    }
  });
};

module.exports.addTimeset = function(req, res) {
  if (!req.user) {
    return res.sendStatus(403);
  }
  var timeset = req.body;
  if (timeset.boat != req.params.boatId) {
    return res.sendStatus(400);
  }
  boatAccess.userCanWriteBoatId(req.user, req.params.boatId)
    .then(function() {
      Timeset.create(timeset, function(err, createdTimeset) {
        if (err) {
          res.sendStatus(500);
        } else {
          res.status(200).json(createdTimeset);
        }
      });
    }).catch(function(err) { 
      res.sendStatus(403); 
    });  
}


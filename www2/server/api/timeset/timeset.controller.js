'use strict'

var Timeset = require('./timeset.model');
var mongoose = require('mongoose');
var boatAccess = require('../boat/access.js');
var _ = require('lodash');

function handleError(res, err) {
  return res.status(500).send(err);
}

// Delete
module.exports.deleteTimeset = function(req, res, next) {
  var search = {
    _id: req.params.timesetId,
    boat: req.params.boatId
  };
  Timeset.remove(search, function(err) {
    if (err) {
      handleError(res, err);
    } else {
      boatAccess.userCanWriteBoatId(req.user, req.params.boatId)
      .then(function() {
        res.sendStatus(200);
      }).catch(function(err) { 
        res.sendStatus(403); 
      });
    }
  });
};

// Get a full list
module.exports.getTimesetsForBoat = function(req, res, next) {
  var boatId = req.params.boatId;
  var search = { boat: boatId };
  Timeset.find(search, function(err, timesets) {
    if (err) {
      res.status(404).end();
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


'use strict'

var SailingSession = require('./session.model');
var mongoose = require('mongoose');
var boatAccess = require('../boat/access.js');
var _ = require('lodash');

function handleError(res, err) {
  return res.status(500).send(err);
}

module.exports.getSessionById = function(req, res, next) {
  var search = {
    _id: req.params.key
  };
  SailingSession.findOne(search, function(err, session) {
    if (err) {
      handleError(res, err);
    } else {
      boatAccess.userCanRead(req.user, session.boat)
      .then(function() {
        res.status(200).json(session);
      })
      .catch(function(err) { res.sendStatus(403); });
    }
  });
};

module.exports.getSessionsForBoat = function(req, res, next) {
  var boatId = req.params.boatId;
  var search = { boat: boatId };
  SailingSession.find(search, function(err, session) {
    if (err) {
      res.status(404).end();
    } else {
      res.status(200).json(session);
    }
  });
};

module.exports.listSessions = function(req, res, next) {
  boatAccess.readableBoats(req)
  .then(function (boats) {
    var boatObjs = _.map(boats, '_id');
    console.warn(boatObjs);
    SailingSession.find({
      boat: {$in : boatObjs},
      trajectoryLength : {$gt : 0.5}
    }, function(err, result) {
      if (err) {
        return res.sendStatus(500);
      }
      res.status(200).json(result);
    });
  })
  .catch(function(err) { handleError(res, err); });
}


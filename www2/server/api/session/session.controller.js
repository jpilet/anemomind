'use strict'

var Session = require('./session.model');
var mongoose = require('mongoose');
var boatAccess = require('../boat/access.js');

module.exports.getSessionById = function(req, res, next) {
  var search = {
    _id: req.params.key
  };
  Session.findOne(search, function(err, session) {
    if (err) {
      next(err);
    } else {
      //boatAccess.userCanReadBoatId(req.user.id, session.boat)
      //.then(function() {
        res.contentType('application/json');
        res.send(JSON.stringify(session));
      //})
      //.catch(function(err) {next(err);});
    }
  });
};

module.exports.getSessionsForBoat = function(req, res, next) {
  var search = {
    boat: req.params.id
  };
  Session.find(search, function(err, session) {
    if (err) {
      next(err);
    } else {
      res.contentType('application/json');
      res.send(JSON.stringify(session));
    }
  });
};

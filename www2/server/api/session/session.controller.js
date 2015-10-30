'use strict'

var Session = require('./session.model');
var mongoose = require('mongoose');

module.exports.getSessionById = function(req, res, next) {
  var search = {
    _id: req.params.key
  };
  Session.findOne(search, function(err, session) {
    if (err) {
      next(err);
    } else {
      res.contentType('application/json');
      res.send(JSON.stringify(session));
    }
  });
};

module.exports.getSessionsForBoat = function(req, res, next) {
  console.log('GET_SESSIONS_FOR_BOAT called with %j', req.id);
  for (var key in req) {
    console.log('Value of %s is %j', key, req[key]);
  }
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

'use strict'

var Summary = require('./summary.model');
var mongoose = require('mongoose');

module.exports.getSummaryForSession = function(req, res, next) {
  var search = {
    curveId: req.params.key
  };
  Summary.findOne(search, function(err, summary) {
    if (err) {
      next(err);
    } else {
      res.contentType('application/json');
      res.send(JSON.stringify(summary));
    }
  });
};

module.exports.getSummariesForBoat = function(req, res, next) {
  var search = {
    boat: req.id
  };
  Summary.find(search, function(err, summary) {
    if (err) {
      next(err);
    } else {
      res.contentType('application/json');
      res.send(JSON.stringify(summary));
    }
  });
};

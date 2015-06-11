'use strict';

var _ = require('lodash');
var Boxexec = require('./boxexec.model');

// Get list of boxexecs
exports.index = function(req, res) {
  Boxexec.find(function (err, boxexecs) {
    if(err) { return handleError(res, err); }
    return res.json(200, boxexecs);
  });
};

// Get a single boxexec
exports.show = function(req, res) {
  Boxexec.findById(req.params.id, function (err, boxexec) {
    if(err) { return handleError(res, err); }
    if(!boxexec) { return res.send(404); }
    return res.json(boxexec);
  });
};

// Creates a new boxexec in the DB.
exports.create = function(req, res) {
  Boxexec.create(req.body, function(err, boxexec) {
    if(err) { return handleError(res, err); }
    return res.json(201, boxexec);
  });
};

// Updates an existing boxexec in the DB.
exports.update = function(req, res) {
  if(req.body._id) { delete req.body._id; }
  Boxexec.findById(req.params.id, function (err, boxexec) {
    if (err) { return handleError(res, err); }
    if(!boxexec) { return res.send(404); }
    var updated = _.merge(boxexec, req.body);
    updated.save(function (err) {
      if (err) { return handleError(res, err); }
      return res.json(200, boxexec);
    });
  });
};

// Deletes a boxexec from the DB.
exports.destroy = function(req, res) {
  Boxexec.findById(req.params.id, function (err, boxexec) {
    if(err) { return handleError(res, err); }
    if(!boxexec) { return res.send(404); }
    boxexec.remove(function(err) {
      if(err) { return handleError(res, err); }
      return res.send(204);
    });
  });
};

function handleError(res, err) {
  return res.send(500, err);
}

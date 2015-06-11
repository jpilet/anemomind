'use strict';

var _ = require('lodash');
var Script = require('./script.model');

// Get list of scripts
exports.index = function(req, res) {
  Script.find(function (err, scripts) {
    if(err) { return handleError(res, err); }
    return res.json(200, scripts);
  });
};

// Get a single script
exports.show = function(req, res) {
  Script.findById(req.params.id, function (err, script) {
    if(err) { return handleError(res, err); }
    if(!script) { return res.send(404); }
    return res.json(script);
  });
};

// Creates a new script in the DB.
exports.create = function(req, res) {
  Script.create(req.body, function(err, script) {
    if(err) { return handleError(res, err); }
    return res.json(201, script);
  });
};

// Updates an existing script in the DB.
exports.update = function(req, res) {
  if(req.body._id) { delete req.body._id; }
  Script.findById(req.params.id, function (err, script) {
    if (err) { return handleError(res, err); }
    if(!script) { return res.send(404); }
    var updated = _.merge(script, req.body);
    updated.save(function (err) {
      if (err) { return handleError(res, err); }
      return res.json(200, script);
    });
  });
};

// Deletes a script from the DB.
exports.destroy = function(req, res) {
  Script.findById(req.params.id, function (err, script) {
    if(err) { return handleError(res, err); }
    if(!script) { return res.send(404); }
    script.remove(function(err) {
      if(err) { return handleError(res, err); }
      return res.send(204);
    });
  });
};

function handleError(res, err) {
  return res.send(500, err);
}
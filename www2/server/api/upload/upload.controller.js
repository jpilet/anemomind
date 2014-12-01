'use strict';

var _ = require('lodash');
var Upload = require('./upload.model');

// Get list of uploads
exports.index = function(req, res) {
  Upload.find(function (err, uploads) {
    if(err) { return handleError(res, err); }
    return res.json(200, uploads);
  });
};

// Get a single upload
exports.show = function(req, res) {
  Upload.findById(req.params.id, function (err, upload) {
    if(err) { return handleError(res, err); }
    if(!upload) { return res.send(404); }
    return res.json(upload);
  });
};

// Creates a new upload in the DB.
exports.create = function(req, res) {
  Upload.create(req.body, function(err, upload) {
    if(err) { return handleError(res, err); }
    return res.json(201, upload);
  });
};

// Updates an existing upload in the DB.
exports.update = function(req, res) {
  if(req.body._id) { delete req.body._id; }
  Upload.findById(req.params.id, function (err, upload) {
    if (err) { return handleError(res, err); }
    if(!upload) { return res.send(404); }
    var updated = _.merge(upload, req.body);
    updated.save(function (err) {
      if (err) { return handleError(res, err); }
      return res.json(200, upload);
    });
  });
};

// Deletes a upload from the DB.
exports.destroy = function(req, res) {
  Upload.findById(req.params.id, function (err, upload) {
    if(err) { return handleError(res, err); }
    if(!upload) { return res.send(404); }
    upload.remove(function(err) {
      if(err) { return handleError(res, err); }
      return res.send(204);
    });
  });
};

function handleError(res, err) {
  return res.send(500, err);
}
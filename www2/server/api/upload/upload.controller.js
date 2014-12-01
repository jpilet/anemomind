'use strict';

var _ = require('lodash');
var Upload = require('./upload.model');

// Get list of uploads
exports.upload = function(req, res) {
  res.json(201, 'ok');
};
'use strict';

var _ = require('lodash');
var Upload = require('./upload.model');

// Get list of uploads
exports.upload = function(req, res) {
  res.status(201).json('ok');
};

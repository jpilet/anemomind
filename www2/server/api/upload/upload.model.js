'use strict';

var mongoose = require('mongoose'),
    Schema = mongoose.Schema;

var UploadSchema = new Schema({
  name: String,
  info: String,
  active: Boolean
});

module.exports = mongoose.model('Upload', UploadSchema);
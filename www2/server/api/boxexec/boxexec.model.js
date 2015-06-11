'use strict';

var mongoose = require('mongoose'),
    Schema = mongoose.Schema;

var BoxExecSchema = new Schema({
  reqCode: String,
  err: String,
  stdout: String,
  stderr: String
});

module.exports = mongoose.model('BoxExec', BoxExecSchema);

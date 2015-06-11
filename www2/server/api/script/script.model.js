'use strict';

var mongoose = require('mongoose'),
    Schema = mongoose.Schema;

var ScriptSchema = new Schema({
  reqCode: String,
  err: String,
  stdout: String,
  stderr: String
});

module.exports = mongoose.model('Script', ScriptSchema);

'use strict';

var mongoose = require('mongoose'),
    Schema = mongoose.Schema;

var ScriptSchema = new Schema({
  name: String,
  info: String,
  active: Boolean
});

module.exports = mongoose.model('Script', ScriptSchema);
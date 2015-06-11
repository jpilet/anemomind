'use strict';

var mongoose = require('mongoose'),
    Schema = mongoose.Schema;

var ScriptSchema = new Schema({
  reqCode: String,
  response: String
});

module.exports = mongoose.model('Script', ScriptSchema);

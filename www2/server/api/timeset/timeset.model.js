'use strict';

/*
  A sailing session and related information
*/

var mongoose = require('mongoose');
var Schema = mongoose.Schema;
  
// See also server/nautical/timesets/TimeSets.h
var TimesetSchema = new Schema({
  _id: Schema.ObjectId,
  boat: Schema.ObjectId,
  begin: Date,
  end: Date,
  type: String,
  data: String,
});

module.exports = mongoose.model('Timeset', TimesetSchema);

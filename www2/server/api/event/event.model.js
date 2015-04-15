'use strict';

var mongoose = require('mongoose'),
    Schema = mongoose.Schema;

var EventSchema = new Schema({
  author: Schema.ObjectId,
  boat: Schema.ObjectId,
  structuredMessage: String,
  comment: String,
  picture: Schema.ObjectId,
  when: Date,
  latitude: Number,
  longitude: Number
});

module.exports = mongoose.model('Event', EventSchema);

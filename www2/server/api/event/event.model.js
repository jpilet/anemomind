'use strict';

var mongoose = require('mongoose'),
    Schema = mongoose.Schema;

// This schema corresponds to
// ANMEvent
// https://github.com/jpilet/anemomind-ios/blob/master/AnemomindApp/ANMEvent.swift#L13
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

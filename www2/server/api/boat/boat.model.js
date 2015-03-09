'use strict';

var mongoose = require('mongoose');
var Schema = mongoose.Schema;

var BoatSchema = new Schema({
  name: String,
  type: String,
  active: Boolean,
  lengthMeters: Number,
  sails: [ String ],
  admins: [ Schema.ObjectId ],
  readers: [ Schema.ObjectId ]
});

module.exports = mongoose.model('Boat', BoatSchema);

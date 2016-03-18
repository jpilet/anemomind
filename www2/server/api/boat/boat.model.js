'use strict';

var mongoose = require('mongoose');
var Schema = mongoose.Schema;

var BoatSchema = new Schema({
  name: String,
  type: String,
  sailNumber: String,
  length: String,
  lengthUnit: {type: String, enum: ['meter', 'feet']},
  sails: [ String ],
  admins: [ Schema.ObjectId ],
  readers: [ Schema.ObjectId ],
  invited: [{
    email: String,
    admin: Boolean
  }],
  anemobox: String,
  firmwareVersion: String
});

module.exports = mongoose.model('Boat', BoatSchema);

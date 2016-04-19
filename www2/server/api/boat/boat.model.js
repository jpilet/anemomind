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
  firmwareVersion: String,

  // if set, no authentication is required to read this boat data.
  // everybody is a reader.
  publicAccess: Boolean
});

module.exports = mongoose.model('Boat', BoatSchema);

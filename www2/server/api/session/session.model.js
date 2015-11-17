'use strict';

/*
  A sailing session and related information
*/

var mongoose = require('mongoose');
var Schema = mongoose.Schema;
  
var SessionSchema = new Schema({
  boat: Schema.ObjectId,
  startTime: Date,
  endTime: Date,
  _id: String,                // same key as the curveId in tiles.model.js
  maxSpeedOverGround: Number, // knots
  maxSpeedOverGroundTime: Date,
  trajectoryLength: Number    // nautical miles

  // TODO
  //maxSpeedOverWater: Number,   // knots
});

module.exports = mongoose.model('SailingSession', SessionSchema);

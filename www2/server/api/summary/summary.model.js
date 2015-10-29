'use strict';

/*
  Summary of a sailing session
*/

var mongoose = require('mongoose');
var Schema = mongoose.Schema;
  
var SummarySchema = new Schema({
  boat: Schema.ObjectId,     
  curveId: String,             // same key as for tiles.model.js
  maxSpeedOverGround: Number,  // knots
  trajectoryLength: Number   // nautical miles

  // TODO
  //maxSpeedOverWater: Number,   // knots
});

module.exports = mongoose.model('Summary', SummarySchema);

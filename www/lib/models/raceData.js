'use strict';

var mongoose = require('mongoose'),
    Schema = mongoose.Schema;
  
/**
 * Upload Schema
 */
var RaceDataSchema = new Schema({
  title: String,
  boatId: String,
  items: [{
    latRad : Number,
    lonRad : Number,
    awaRad: Number,
    awsMps: Number,
    externalTwaRad: Number,
    externalTwsMps: Number,
    twdirRad: Number,
    twsMps: Number,
    gpsBearingRad: Number,
    gpsSpeedMps: Number,
    magHdgRad: Number,
    timeMs: Number,
    watSpeedMps: Number
  }]
});

/**
 * Virtuals
 */


    
/**
 * Validations
 */




/**
 * Pre-save hook
 */




/**
 * Methods
 */

module.exports = mongoose.model('RaceData', RaceDataSchema);

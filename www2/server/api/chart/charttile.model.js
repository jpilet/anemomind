'use strict';

var mongoose = require('mongoose');
var Schema = mongoose.Schema;
  
var ChartTileSchema = new Schema({
    boat: Schema.ObjectId,
    zoom: Number,
    tileno: Number,
    what: String,
    source: String,

    samples: {
      count: Number,
      mean: Number,
      stdev: Number
    }
  });

module.exports = mongoose.model('ChartTile', ChartTileSchema);

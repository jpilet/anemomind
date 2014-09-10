'use strict';

var mongoose = require('mongoose');
var Schema = mongoose.Schema;
  
var TileSchema = new Schema({
    boat: String,
    key: String,
    startTime: Date,
    endTime: Date,
    created: Date,
    curves: [{
      curveId: String,

      // Sequence of connected points that are part of 'curveId'.
      // the curve might begin or continue on other tiles.
      points: [{
        time: Date,

        // a 2-D array of OSM coordinates between 0 and 1.
        // see
        // http://wiki.openstreetmap.org/wiki/Slippy_map_tilenames#C.2FC.2B.2B
        pos: [Number],
        gpsBearing: Number,  // in degrees
        gpsSpeed: Number,  // in knots
        awa: Number,  // in degrees
        aws: Number,  // in knots
        magHdg: Number,  // in degrees
        watSpeed: Number,  // in knots
        externalTwa: Number,  // in degrees, relative to boat orientation
        externalTws: Number,  // in knots
        twdir: Number,  // in degrees, relative to north
        tws: Number  // in knots
        }]
      }
    ]
  });

module.exports = mongoose.model('Tile', TileSchema);

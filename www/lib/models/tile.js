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
      points: [{
        time: Date,
        pos: [Number],
        gpsBearing: Number,
        gpsSpeed: Number,
        awa: Number,
        aws: Number,
        magHdg: Number,
        watSpeed: Number,
        externalTwa: Number,
        externalTws: Number,
        twdir: Number,
        tws: Number
        }]
      }
    ]
  });

module.exports = mongoose.model('Tile', TileSchema);

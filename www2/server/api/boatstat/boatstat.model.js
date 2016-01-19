'use strict';

var mongoose = require('mongoose');
var Schema = mongoose.Schema;

var BoatstatSchema = new Schema({
  vmgtable: [ { tws: Number, up: Number, down: Number } ]
});

module.exports = mongoose.model('Boatstat', BoatstatSchema);

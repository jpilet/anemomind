'use strict';

var mongoose = require('mongoose');
var Schema = mongoose.Schema;
  
var ChartSourceSchema = new Schema({
    channels: { }, // this basically tells Mongoose not to try to verify the object
  });

module.exports = mongoose.model('ChartSource', ChartSourceSchema);

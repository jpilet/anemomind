'use strict';

var mongoose = require('mongoose'),
    Schema = mongoose.Schema;

var PacketSchema = new Schema({
  name: String,
  info: String,
  active: Boolean
});

module.exports = mongoose.model('Packet', PacketSchema);
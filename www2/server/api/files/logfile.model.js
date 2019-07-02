'use strict';

const mongoose = require('mongoose');
const Schema = mongoose.Schema;

const LogFileSchema = new Schema({
  name: { type: String, required: true },
  boat: { type: Schema.ObjectId, required: true },

  start: Date,
  end: Date,
  duration_sec: Number,
  uploadedBy: Schema.ObjectId,
  uploadDate: Date,
  type: {
    type: String,
    enum: [ 'log', 'ESA Polar', 'unknown' ],
    required: true
  },
  data: String, // "awa,aws,gpsSpeed"

  // Size in bytes
  size: Number,
 
  // If something goes wrong, we can report an error here.
  error: String,

  // optional. Should be filled by processBoatLogs with a timestamp representing
  // when the file has been processed.
  processed: Date,
});

module.exports = mongoose.model('LogFile', LogFileSchema);

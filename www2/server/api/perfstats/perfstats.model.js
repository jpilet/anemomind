
const mongoose = require('mongoose');
const Schema = mongoose.Schema;

const PerfStatSchema = new Schema({
  boat: Schema.ObjectId,
  name: String,
  urlName: String, // Same as name, but containing only easy chars.
  type: String,

  esaDataRegime: [ { tws: Number, twa: Number, boatSpeed: Number } ], 
  polar: [ { tws: Number, twa: Number, boatSpeed: Number } ], 
  esaVmgPoints: [ [Number ] ], 

  status: { type: String, enum: [ 'ready', 'in-progress', 'processing', 'failed' ] },
  lastStateChange: Date,
  error: String,

  // Reply from ESA server.
  // Can contain: { results: "url", "esa": "url", "stats": "url" }
  analyzeResult: Object,

  created: Date,

  start: Date,
  end: Date
});

module.exports = mongoose.model('PerfStat', PerfStatSchema);

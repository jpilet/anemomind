
const mongoose = require('mongoose');
const Schema = mongoose.Schema;

const PerfStatSchema = new Schema({
  boat: Schema.ObjectId,
  name: String,
  type: String,
  esaDataRegime: [ { tws: Number, twa: Number, boatSpeed: Number } ], 
  polar: [ { tws: Number, twa: Number, boatSpeed: Number } ], 
  esaVmgPoints: [ [Number ] ], 
});

module.exports = mongoose.model('PerfStat', PerfStatSchema);

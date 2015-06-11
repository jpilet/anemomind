'use strict';

var mongoose = require('mongoose'),
    Schema = mongoose.Schema;

var BoxExecSchema = new Schema({

  // Request-related:
  boatId: String,
  boxId: String,
  type: String,
  script: String,
  
  complete: Boolean,

  // Response-releated
  err: String,
  stdout: String,
  stderr: String
});

module.exports = mongoose.model('BoxExec', BoxExecSchema);

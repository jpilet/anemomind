'use strict';

var mongoose = require('mongoose'),
    Schema = mongoose.Schema;
  
/**
 * Upload Schema
 */
var UploadSchema = new Schema({
  user: String,
  file: String,
  title: String
});

/**
 * Virtuals
 */


    
/**
 * Validations
 */




/**
 * Pre-save hook
 */




/**
 * Methods
 */

module.exports = mongoose.model('Upload', UploadSchema);
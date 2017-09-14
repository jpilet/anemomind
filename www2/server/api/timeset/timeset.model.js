'use strict';

/*
  A sailing session and related information
*/

var mongoose = require('mongoose');
var Schema = mongoose.Schema;
  
// See also server/nautical/timesets/TimeSets.h
var TimesetSchema = new Schema({
  boat: Schema.ObjectId, // The boat it is associated with.
  lower: Date,  // Where it starts.
  upper: Date,    // Where it ends.
  type: String, // Typically some operation, such as 'delete', 'ignore', 'merge', 'split'.
  data: String, // Any data that we would like to associate with it. Should we constrain
                // it to be a string, or would it be better if it were more freeform?
  creationTime: Date // When it was created. This lets 
                     // us replay the operations in chronological order.
});

module.exports = mongoose.model('Timeset', TimesetSchema);

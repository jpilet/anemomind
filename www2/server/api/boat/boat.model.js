'use strict';

var mongoose = require('mongoose');
var Schema = mongoose.Schema;
var Q = require('q');

var BoatSchema = new Schema({
  name: String,
  type: String,
  sailNumber: String,
  length: String,
  lengthUnit: {type: String, enum: ['meter', 'feet']},
  sails: [ String ],
  admins: [ Schema.ObjectId ],
  readers: [ Schema.ObjectId ],
  invited: [{
    email: String,
    admin: Boolean
  }],
  anemobox: String,
  firmwareVersion: String,

  // if set, no authentication is required to read this boat data.
  // everybody is a reader.
  publicAccess: Boolean
});


//
// load boat with photos and comments 
BoatSchema.statics.findWithPhotosAndComments=function (query,callback) {
  var promise=new Promise((resolve, reject) => {

  var Events=mongoose.model('Event');
  var results=[];
  var filteredEvents=[];

  this.find(query, function (err, boats){
    if(err){
      return reject(err);
    }
    if(!boats.length){
      return resolve([]);
    }

    //
    // prepare query for events
    var boats_id=boats.map(function(b) {
      return b._id;
    });

    Events.collectSocialDataByBoat(boats_id,function(err,eventsByBoat) {
      if(err){
        return reject(err);
      }
      boats.forEach(function(boat) {

        // get plain js object
        var singleBoat=boat.toObject();

        //
        // get social data for this boat 
        var event=eventsByBoat.filter(function(event) {
          return event._id.equals(boat._id);
        })[0];

        //
        // if no data, 
        if(!event){ return results.push(singleBoat);}

        //
        // keep all photos to access from session widget
        singleBoat.photos=event.photos;

        //
        // keep all comments to access from session widget
        singleBoat.comments=event.comments;
        results.push(singleBoat);
      });



      // new API coldrun 10-20 ms hotrun 5-10ms
      // old API coldrun 80-90 ms hotrun 60-80ms
      // console.log('----------------- collect',(Date.now()-time)/1000);
      resolve(results);
    });
  });
  });

  if(callback){
    promise.then((boats) => { callback(undefined, boats); });
    promise.catch((err) => { callback(err); });
  }

  return promise;
}

module.exports = mongoose.model('Boat', BoatSchema);

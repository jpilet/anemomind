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
// load boat with resume, last photos and comments 
BoatSchema.statics.findWithPhotosAndComments=function (query,callback) {
  var Events=mongoose.model('Event');
  var promise=new mongoose.Promise;
  var results=[];
  var filteredEvents=[];

  if(callback){promise.addBack(callback);}

  this.find(query, function (err, boats){
    if(err){
      return promise.reject(err);
    }
    if(!boats.length){
      return promise.resolve(null,[]);
    }

    //
    // prepare query for events
    var boats_id=boats.map(function(b) {
      return b._id;
    });

    Events.collectSocialDataByBoat(boats_id,function(err,eventsByBoat) {
      if(err){
        return promise.reject(err);
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
        // clean photos (TODO this could be done by the aggregate, check mongoose version !!)
        singleBoat.photos=event.photos.filter(function(photo) {
          return photo.src;
        });

        //
        // clean comments (TODO this could be done by the aggregate, check mongoose version !!)
        singleBoat.comments=event.comments.filter(function (comment) {
          return comment.txt;
        });
        results.push(singleBoat);
      });



      // new API coldrun 10-20 ms hotrun 5-10ms
      // old API coldrun 80-90 ms hotrun 60-80ms
      // console.log('----------------- collect',(Date.now()-time)/1000);
      promise.resolve(null,results);
    });

  });

  return promise;
}

module.exports = mongoose.model('Boat', BoatSchema);

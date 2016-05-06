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
// load boat with photos and comments from events
BoatSchema.statics.findWithPhotosAndComments=function (query,callback) {
  var Events=mongoose.model('Event'), 
      promise=new mongoose.Promise, 
      results=[],
      singleBoat={},
      filteredEvents=[];

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

    //
    // map all (comments & photos) with each boat
    Events.find({boat:{$in:boats_id}},function (err,events) {
      if(err){
        return promise.reject(err);
      }

      boats.forEach(function(boat) {
        // get plain js object
        singleBoat=boat.toObject();

        //
        // filter event for this boat
        filteredEvents=events.filter(function(event) {
          return event.boat.equals(boat._id);
        });

        //
        // collect photos 
        singleBoat.photos=filteredEvents.filter(function(e) {
          return e.photo;
        }).map(function(event) {
          return {src:event.photo,when:event.when};
        });

        //
        // collect comments
        singleBoat.comments=filteredEvents.filter(function(e) {
          return e.comment;
        }).map(function(event) {
          return {text:event.comment,when:event.when};
        });
        results.push(singleBoat);
      })

      promise.resolve(null,results);
    });

  });

  return promise;
}

module.exports = mongoose.model('Boat', BoatSchema);

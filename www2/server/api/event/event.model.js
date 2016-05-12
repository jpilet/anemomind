'use strict';

var mongoose = require('mongoose'),
    Schema = mongoose.Schema;

// This schema corresponds to
// ANMEvent
// https://github.com/jpilet/anemomind-ios/blob/master/AnemomindApp/ANMEvent.swift#L13
var EventSchema = new Schema({
  author: Schema.ObjectId,
  boat: Schema.ObjectId,
  structuredMessage: String,
  comment: String,
  photo: String,
  when: Date,
  latitude: Number,
  longitude: Number
});

//
// This API collect social content (photo and comment) that belongs to a boat
//  - boatsId mondatory, define a list of boat as object id array
//  - callback optional, you can choose on callback or promise
EventSchema.statics.collectSocialDataByBoat=function (boatsId,callback) {
  var promise=new mongoose.Promise;
  if(callback){promise.addBack(callback);}

  if(!boatsId||!boatsId.length){
  	return promise.reject(new Error("boatsId should not be null or empty!"))
  }
	//
	// filter relevants events
	var select={
	  boat:{
	  	$in:boatsId
	  }, 
	  $or:[
	  	{'photo': {$exists:true}},{'comment':{$exists:true}}
	  ]
	};

	//
	// group photo & comment by boats
	// It seems that $cond is not working with mongoose <4.0 !
	// TODO $addToSet & limit array size, use { $each: [$elem], $slice: -3 }
	this.aggregate([
   {$match: select },
   {$project:{
       photo:1,
       comment:1,
       boat:1,
       author:1,
       when:1
   }},
   {$group:
       {
         _id:"$boat",
         photos:{
          $addToSet:{src:"$photo",when:"$when",who:"$author"}
         },
         comments:{
          $addToSet:{txt:"$comment",when:"$when",who:"$author"}
         }
       }
   },
   {$unwind: '$photos' },
   {$match:{'photos.src':{$exists:true}}},
   {$unwind: '$comments' },
   {$match:{'comments.txt':{$exists:true}}},
   {$group:
       {
         _id:"$_id",
         photos:{
          $addToSet:"$photos"
         },
         comments:{
          $addToSet:"$comments"
         },
       }
   }],function(err,boats) {
   		if(err){
   			return promise.reject(err);
   		}
      promise.resolve(err,boats);

   });
}


module.exports = mongoose.model('Event', EventSchema);

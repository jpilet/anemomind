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

EventSchema.statics.collectSocialDataByBoat=function (boatsId,callback) {
  var promise=new mongoose.Promise;
  if(callback){promise.addBack(callback);}

	//
	// filter relevants events
	var select={
	  boat:{
	  	$in:boatsId||[]
	  }, 
	  $or:[
	  	{'photo': {$exists:true}},{'comment':{$exists:true}}
	  ]
	};

	//
	// group photo & comment by boats
	// It seems that $cond is not working with mongoose <4.0 !
	this.aggregate(
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
          $addToSet:{
            "$cond": [
                { "$ne": [ "$photo", undefined ] },
                {src:"$photo",when:"$when",who:"$author"},undefined
            ]
          }
         },
         comments:{
          $addToSet:{
            "$cond": [
                { "$ne": [ "$comment", undefined ] },
                {txt:"$comment",when:"$when",who:"$author"},undefined
            ]
          }
         }
       }
   },function(err,boats) {
   		if(err){
   			return promise.reject(err);
   		}
      promise.resolve(err,boats);

   });
}


module.exports = mongoose.model('Event', EventSchema);

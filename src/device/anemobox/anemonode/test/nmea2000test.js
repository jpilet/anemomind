var nmea2000 = require('../components/nmea2000.js');
var assert = require('assert');

var exampleMessage = {"ts_sec":1498141174,
	"ts_usec":139588,"id":167576077,"ext":true,
	"data":{"type":"Buffer","data":[42,159,2,29,20,250,255,255]}};  
	
var ser = nmea2000.serializeMessage(exampleMessage);
assert(!nmea2000.getError(ser));
var deser = nmea2000.deserializeMessage(exampleMessage);


console.log("Serialized: %j", ser);
console.log("Deserialized: %j", deser);





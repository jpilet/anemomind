var nmea2000 = require('../components/nmea2000.js');
var assert = require('assert');

var exampleMessage = {"ts_sec":1498141174,
	"ts_usec":139588,"id":167576077,"ext":true,
	"data": new Buffer([42,159,2,29,20,250,255,255])};  
	
var ser = nmea2000.serializeMessage(exampleMessage);
console.log("ERROR: %j", nmea2000.getError(ser));
assert(!nmea2000.getError(ser))
var deser = nmea2000.deserializeMessage(ser);

assert(deser.id == exampleMessage.id);
assert(deser.ts_sec == exampleMessage.ts_sec);
assert(deser.ts_usec == exampleMessage.ts_usec);
assert(deser.data.equals(exampleMessage.data));

console.log("Serialized: %j", ser);
console.log("Deserialized: %j", deser);

nmea2000.startCanSource();

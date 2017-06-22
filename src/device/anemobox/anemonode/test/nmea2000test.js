var canutils = require('../components/canutils.js');
var assert = require('assert');
var nmea2000 = require('../components/nmea2000.js');

var exampleMessage = {"ts_sec":1498141174,
	"ts_usec":139588,"id":167576077,"ext":true,
	"data": new Buffer([42,159,2,29,20,250,255,255])};  
	
var ser = canutils.serializeMessage(exampleMessage);
console.log("ERROR: %j", canutils.getError(ser));
assert(!canutils.getError(ser))
var deser = canutils.deserializeMessage(ser);

assert(deser.id == exampleMessage.id);
assert(deser.ts_sec == exampleMessage.ts_sec);
assert(deser.ts_usec == exampleMessage.ts_usec);
assert(deser.data.equals(exampleMessage.data));

console.log("Serialized: %j", ser);
console.log("Deserialized: %j", deser);

var cs = new nmea2000.CanSource(function(msg) {
  console.log("Got this message: %j", msg);
});

var data = ["aaaa\nbbb\nccc", "dd\nee"];
var f = canutils.catSplit("\n")(function(dst, x) {dst.push(x); return dst;});
var split = data.reduce(f, []);
assert(split.length == 3);
assert(split[2] == "cccdd");

cs.start();

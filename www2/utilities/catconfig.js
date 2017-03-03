// Example usage:
// node utilities/catconfig.js %j x.mongo.uri

var assert = require('assert');

function isValidMode(x) {
  var validModes = ["development", "production", "test"];
  for (var i = 0; i < validModes.length; i++) {
    if (validModes[i] == x) {
      return true;
    }    
  }
  console.log("Invalid mode: '%s'", x);
  console.log("Valid modes are %j", validModes);
  return false;
}
assert(isValidMode(process.env.NODE_ENV));

var cmdArgs = process.argv.slice(2);
var fmt = cmdArgs[0];
assert(fmt == '%j' || fmt == '%s');
var expr = cmdArgs[1];

var x = require('../server/config/environment/index.js');
console.log(fmt, eval(expr));

// Example usage:
// node utilities/catconfig.js development %j x.mongo.uri

var assert = require('assert');

function isValidMode(x) {
  var validModes = ["development", "production", "test"];
  for (var i = 0; i < validModes.length; i++) {
    if (validModes[i] == x) {
      return true;
    }    
  }
  return false;
}

var backup = process.env.NODE_ENV;
var cmdArgs = process.argv.slice(2);
var mode = cmdArgs[0];
if (!isValidMode(mode)) {
  throw new Error("Invalid mode: '" + mode + "'");  
}
var fmt = cmdArgs[1];
assert(fmt == '%j' || fmt == '%s');
var expr = cmdArgs[2];


process.env.NODE_ENV = mode;

var x = require('../server/config/environment/index.js');
console.log(fmt, eval(expr));

process.env.NODE_ENV = backup;

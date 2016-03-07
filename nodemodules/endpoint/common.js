var bigint = require('./bigint.js');
var assert = require('assert');
var Q = require('q');

module.exports.ack = 127;
module.exports.logfile = 128;
module.exports.scriptRequest = 129;
module.exports.scriptResponse = 130;
module.exports.files = 131;
module.exports.bundle = 134;


function ResultArray(n, cb) {
  this.dst = new Array(n);
  this.counter = 0;
  this.cb = cb;
  this.tryToDeliver();
}

ResultArray.prototype.tryToDeliver = function() {
  if (this.counter == this.dst.length) {
    this.cb(null, this.dst);
    this.cb = undefined;
  }
}

ResultArray.prototype.makeSetter = function(index) {
  var self = this;
  return function(err, value) {
    if (err) {
      if (self.cb) {
	self.cb(err);
      }
      self.cb = undefined;
    } else {
      if (self.counter >= self.dst.length) {
	throw new Error('MULTIPLE DELIVERY OF RESULT!!! THIS IS A PROGRAMMING ERROR');
      }
      self.dst[index] = value;
      self.counter++;
      self.tryToDeliver();
    }
  };
}

function isString(x) {
  return typeof x == 'string';
}

function isIdentifier(x) {
  if (typeof x == 'string') {
    // numbers and digits and some simple characters. At least one character.
    return !!x.match(/^[\w]+$/); 
  }
  return false;
}

function isCounter(x) {
  return bigint.isBigInt(x);
}

function isValidEndpointName(x) {
  return isIdentifier(x);
}

function isPacket(x) {
  if (typeof x == 'object') {
    return x.src && x.dst && x.label && x.seqNumber && x.data;
  }
  return false;
}

function isObjectWithFields(x, fields) {
  if (typeof x == 'object') {
    for (var i = 0; i < fields.length; i++) {
      if (!x.hasOwnProperty(fields[i])) {
	return false;
      }
    }
    return true;
  }
  return false;
}


/*
    How to get the names of function parameters:
    
      http://stackoverflow.com/questions/1007981/how-to-get-function-parameter-names-values-dynamically-from-javascript

    Can be used to validate that the methods of a endpoint implements
    this specification.
*/
var STRIP_COMMENTS = /((\/\/.*$)|(\/\*[\s\S]*?\*\/))/mg;
var ARGUMENT_NAMES = /([^\s,]+)/g;
function getParamNames(func) {
  var fnStr = func.toString().replace(STRIP_COMMENTS, '');
  var result = fnStr.slice(fnStr.indexOf('(')+1, fnStr.indexOf(')')).match(ARGUMENT_NAMES);
  if(result === null)
     result = [];
  return result;
}

function makeValuePasser(value, cb) {
  assert(typeof cb == 'function');
  return function(err) {
    cb(err, value);
  };
}

function argsToArray(args) {
  return Array.prototype.slice.call(args);
}

function withException(op, cb) {
  try {
    op();
  } catch (e) {
    cb(e);
  }
}

function fwrap(x) {
  return function() {
    return x;
  }
}

function pfwrap(x) {
  return Q.promised(fwrap(x));
}

module.exports.isCounter = isCounter; 
module.exports.isIdentifier = isIdentifier;
module.exports.isValidEndpointName = isValidEndpointName;
module.exports.isPacket = isPacket;
module.exports.isString = isString;
module.exports.isObjectWithFields = isObjectWithFields;
module.exports.getParamNames = getParamNames;
module.exports.ResultArray = ResultArray;
module.exports.makeValuePasser = makeValuePasser;
module.exports.argsToArray = argsToArray;
module.exports.withException = withException;
module.exports.fwrap = fwrap;
module.exports.pfwrap = pfwrap;

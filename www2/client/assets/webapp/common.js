export const ack = 127;
export const logfile = 128;
export const scriptRequest = 129;
export const scriptResponse = 130;
export const files = 131;
export const firstPacket = 132;
export const remainingPacket = 133;

export function ResultArray(n, cb) {
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

export function isString(x) {
  return typeof x == 'string';
}

export function isIdentifier(x) {
  if (typeof x == 'string') {
    // numbers and digits and some simple characters. At least one character.
    return !!x.match(/^[\w]+$/); 
  }
  return false;
}

export function isCounter(x) {
  return bigint.isBigInt(x);
}

export function isValidEndpointName(x) {
  return isIdentifier(x);
}

export function isPacket(x) {
  if (typeof x == 'object') {
    return x.src && x.dst && x.label && x.seqNumber && x.data;
  }
  return false;
}

export function isObjectWithFields(x, fields) {
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
export function getParamNames(func) {
  var fnStr = func.toString().replace(STRIP_COMMENTS, '');
  var result = fnStr.slice(fnStr.indexOf('(')+1, fnStr.indexOf(')')).match(ARGUMENT_NAMES);
  if(result === null)
     result = [];
  return result;
}

export function makeValuePasser(value, cb) {
  //assert(typeof cb == 'function');
  return function(err) {
    cb(err, value);
  };
}

export function argsToArray(args) {
  return Array.prototype.slice.call(args);
}

export function withException(op, cb) {
  try {
    op();
  } catch (e) {
    cb(e);
  }
}

export function fwrap(x) {
  return function() {
    return x;
  }
}

//function pfwrap(x) { return Q.promised(fwrap(x)); }

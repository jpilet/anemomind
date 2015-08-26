/*

  This file specifies what
  operations a endpoint interface should support.
  It applies to the direct interaction with a endpoint
  object, a remote endpoint over HTTP, or a remote endpoint
  over bluetooth.

  The specification in this file serves two purposes:
   * It can be used to generate a endpoint API
   * It can be used to check that implementations
     conform with this specification.
  
  Types used for:
    * 'hex' : An integer represented as a hexadecimal number
    * 'buffer' : A buffer. With JSON, it will be converted
      to a string of hexadecimal numbers. With msgpack,
      it will be encoded to a buffer.
    * 'any' : A general object. With JSON it will be encoded
      as a JSON object using the 'json-buffer' library.
      
      With msgpack, it will be encoded
      as a binary buffer.
      
    * The usual ones used in MongoDB schemas:
      String, Boolean, Number, etc.

*/  
var assert = require('assert');
var common = require('./common.js');
var util = require('util');

/*

  METHODS

*/
var getParamNames = common.getParamNames;

function isArrayTypeSpec(x) {
    if (Array.isArray(x)) {
	for (var i = 0; i < x.length; i++) {
	    if (!isValidTypeSpec(x[i])) {
		return false;
	    }
	}
	return true;
    }
    return false;
}

function isValidTypeSpecSub(x) {
    return (x == String) || (x == Boolean)
	|| (x == 'hex') || (x == 'buffer')
	|| (x == Number) || (x == 'any')
	|| (x == null) || isArrayTypeSpec(x)
	|| (x == undefined);
}

function isValidTypeSpec(x) {
    var x = isValidTypeSpecSub(x);
    assert(x);
    return x;
}

var errorTypes = ['any', null];

function isValidArg(x) {
    if (typeof x == 'object') {
	var keys = Object.keys(x);
	if (keys.length == 1) {
	    var key = keys[0];
	    if (key == 'thisEndpoint') {
		console.log('This parameter name is reserved');
		return false;
	    }
	    return isValidTypeSpec(x[key]);
	}
	return false;
    }
    return false;
}


function getArgName(x) {
    var keys = Object.keys(x);
    assert.equal(keys.length, 1);
    return keys[0];
}

function getArgTypeSub(x) {
    if (isArrayTypeSpec(x)) {
	return x[0];
    }
    return x;
}

function getArgType(x) {
    var name = getArgName(x);
    return getArgTypeSub(x[name]);
}


function getArgNames(args) {
    return args.map(getArgName);
}

function areValidArgs(x) {
    if (x == undefined) {
	return false;
    }
    
    if (typeof x.length == 'number') {
	for (var i = 0; i < x.length; i++) {
	    if (!isValidArg(x[i])) {
		return false;
	    }
	}
	return true;
    }
    console.log('Invalid arguments spec: %j', x);
    return false;
}

function isValidSpec(x) {
    return areValidArgs(x.input) && areValidArgs(x.output);
}

function MethodSchema(spec) {
    if (!isValidSpec(spec)) {
	console.log('Bad spce: %j', spec);
	throw new Error('Bad spec');
    }

    // Just copy the fields.
    for (x in spec) {
	this[x] = spec[x];
    }
}

MethodSchema.prototype.isValidMethod = function(x) {
    if (typeof x == 'function') {
	var expectedArgs = getArgNames(this.input);
	var actualArgs = getParamNames(x);
	if (actualArgs.length == 0) {
	    return true;
	} else {
	    if (actualArgs.length - 1 == expectedArgs.length) {
		for (var i = 0; i < expectedArgs.length; i++) {
		    var a = expectedArgs[i];
		    var b = actualArgs[i];
		    if (a != b) {
			console.log('Validation of method failed:');
			console.log(
			    ('Argument mismatch between %j and %j '
			     + 'in %j and %j, respectively'),
			    a, b, expectedArgs, actualArgs);
			return false;
		    }
		}
		return true;
	    }
	    console.log('Argument mismatch between %j and %j',
			expectedArgs, actualArgs);
	    return false;
	}
    }
    return false;
}


/*

  THE ENDPOINT SCHEMA

 */
function EndpointSchema(methods) {
    for (methodName in methods) {
	assert(methods[methodName] instanceof MethodSchema);
	methods[methodName].name = methodName;
    }
    
    this.methods = methods;
}

// Test if x conforms with the endpoint schema.
EndpointSchema.prototype.isValidEndpoint = function(x) {
    for (methodName in this.methods) {
	if (!this.methods[methodName].isValidMethod(x[methodName])) {
	    return false;
	}
    }
    return true;
}

function listInputs(argSpecs, args) {
  var n = Math.min(argSpecs.length, args.length);
  var s = '';
  for (var i = 0; i < n; i++) {
    s += util.format(getArgName(argSpecs[i]) + '=%j', args[i]);
    if (i < n-1) {
      s += ' ';
    }
  }
  return s;
}

function shortenToMaxLength(s, maxLength) {
  if (s.length <= maxLength) {
    return s;
  } else {
    return s.substring(0, maxLength-3) + '...';
  }
}

function makeVerboseMethod(self, methodName, methodSpec, method) {
  return function() {
    var allArgs = common.argsToArray(arguments);
    var last = allArgs.length - 1;
    var args = allArgs.slice(0, last);
    var cb = allArgs[last];
    assert(typeof cb == 'function');
    assert(typeof method == 'function');
    method.apply(self, args.concat([function(err, output) {
      var s = self.name + '.' + methodName + '(' + listInputs(methodSpec.input, args) + '): ';
      if (err) {
        s += util.format('FAILED with '+ err);
      } else {
        s += util.format('%j', output);
      }
      console.log(shortenToMaxLength(s, 3000));
      cb(err, output);
    }]));
  }
}

EndpointSchema.prototype.makeVerbose = function(ep) {
  for (method in this.methods) {
    ep[method] = makeVerboseMethod(ep, method, this.methods[method], ep[method]);
  }
}


module.exports.errorTypes = errorTypes;
module.exports.MethodSchema = MethodSchema;
module.exports.EndpointSchema = EndpointSchema;
module.exports.getArgName = getArgName;
module.exports.getArgType = getArgType;
module.exports.isValidHttpMethod = function(x) {
    return x == 'get' || x == 'post';
}

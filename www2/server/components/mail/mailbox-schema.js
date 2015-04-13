/*

  This file specifies what
  operations a mailbox interface should support.
  It applies to the direct interaction with a mailbox
  object, a remote mailbox over HTTP, or a remote mailbox
  over bluetooth.

  The specification in this file serves two purposes:
   * It can be used to generate a mailbox API
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

/*

  METHODS

*/
var methods = {};


/*
    How to get the names of function parameters:
    
      http://stackoverflow.com/questions/1007981/how-to-get-function-parameter-names-values-dynamically-from-javascript

    Can be used to validate that the methods of a mailbox implements
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
	    if (key == 'thisMailbox') {
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
    assert(keys.length == 1);
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

// Below follows specifications of what methods every
// method of a mailbox object should support:
methods.setForeignDiaryNumber = new MethodSchema({
    httpMethod:'get',
    input: [
	{otherMailbox: 'hex'},
	{newValue: 'hex'}
    ],
    output: [
	{err: errorTypes},
    ]
});

methods.getFirstPacketStartingFrom = new MethodSchema({
    httpMethod:'get',
    input: [
	{diaryNumber: 'hex'},
	{lightWeight: Boolean},
    ],
    output: [
	{err: errorTypes},
	{packet: 'any'}
    ]
});

methods.handleIncomingPacket = new MethodSchema({
    httpMethod:'post',
    input: [
	{packet: 'any'}
    ],
    output: [
	{err: errorTypes}
    ]
});

methods.isAdmissible = new MethodSchema({
    httpMethod:'get',
    input: [
	{src: 'hex'},
	{dst: 'hex'},
	{seqNumber: 'hex'}
    ],
    output: [
	{err: errorTypes},
	{p: Boolean}
    ]
});

methods.getForeignDiaryNumber = new MethodSchema({
    httpMethod:'get',
    input: [
	{otherMailbox: 'hex'}
    ],
    output: [
	{err: errorTypes},
	{diaryNumber: 'hex'}
    ]
});

methods.getForeignStartNumber = new MethodSchema({
    httpMethod:'get',
    input: [
	{otherMailbox: 'hex'}
    ],
    output: [
	{err: errorTypes},
	{diaryNumber: 'hex'}
    ]
});

methods.getMailboxName = new MethodSchema({
    httpMethod:'get',
    input: [],
    output: [
	{err: errorTypes},
	{mailboxName: 'hex'}
    ]
});

methods.reset = new MethodSchema({
    httpMethod:'get',
    input: [],
    output: [
	{err: errorTypes}
    ]
});

methods.sendPacket = new MethodSchema({
    httpMethod: 'post',
    input: [
	{dst: 'hex'},
	{label: Number},
	{data: 'buffer'}
    ],
    output: [
	{err: errorTypes}
    ]
});

methods.getTotalPacketCount = new MethodSchema({
    httpMethod: 'get',
    input: [],
    output: [
	{err: errorTypes},
	{count: Number}
    ]
});








/*

  THE MAILBOX SCHEMA

 */
function MailboxSchema(methods) {
    for (methodName in methods) {
	assert(methods[methodName] instanceof MethodSchema);
	methods[methodName].name = methodName;
    }
    
    this.methods = methods;
}

// Test if x conforms with the mailbox schema.
MailboxSchema.prototype.isValidMailbox = function(x) {
    for (methodName in this.methods) {
	if (!this.methods[methodName].isValidMethod(x[methodName])) {
	    return false;
	}
    }
    return true;
}

module.exports = new MailboxSchema(methods);
module.exports.getArgName = getArgName;
module.exports.getArgType = getArgType;

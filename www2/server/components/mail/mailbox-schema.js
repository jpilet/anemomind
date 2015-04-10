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
    * 'binary' : A buffer. With JSON, it will be converted
      to a string of hexadecimal numbers. With msgpack,
      it will be encoded to a buffer.
    * 'any' : A general object. With JSON it will be encoded
      as a JSON object using the 'json-buffer' library.
      
      With msgpack, it will be encoded
      as a binary buffer.
      
    * The usual ones used in MongoDB schemas:
      String, Boolean, Number, etc.

    How to get the names of function parameters:
    
      http://stackoverflow.com/questions/1007981/how-to-get-function-parameter-names-values-dynamically-from-javascript

    Can be used to validate that the methods of a mailbox implements
    this specification.
*/  
var assert = require('assert');




/*

  METHODS

*/
var methods = {};


// Copied from stack overflow
var STRIP_COMMENTS = /((\/\/.*$)|(\/\*[\s\S]*?\*\/))/mg;
var ARGUMENT_NAMES = /([^\s,]+)/g;
function getParamNames(func) {
  var fnStr = func.toString().replace(STRIP_COMMENTS, '');
  var result = fnStr.slice(fnStr.indexOf('(')+1, fnStr.indexOf(')')).match(ARGUMENT_NAMES);
  if(result === null)
     result = [];
  return result;
}



function isValidTypeSpec(x) {
    return (x == String) || (x == Boolean)
	|| (x == 'hex') || (x == 'buffer')
	|| (x == Number) || (x == 'any') || (x == null);
}

function isValidArg(x) {
    if (typeof x == 'object') {
	var keys = Object.keys(x);
	if (keys.length == 1) {
	    var key = keys[0];
	    return isValidTypeSpec(x[key]);
	}
	return false;
    }
    return false;
}

function areValidArgs(x) {
    if (x.length) {
	for (var i = 0; i < x.length; i++) {
	    if (!isValidArg(x[i])) {
		return false;
	    }
	}
	return true;
    }
    return false;
}

function isValidSpec(x) {
    return areValidArgs(x.inputs) && areValidArgs(x.outputs);
}

function MethodSchema(spec) {
    this.spec = spec;
}

MethodSchema.prototype.isValidMethod = function(x) {
    if (typeof x == 'function') {
	return true;
    }
    return false;
}

// Below follows specifications of what methods every
// method of a mailbox object should support:
methods.setForeignDiaryNumber = new MethodSchema({
    input: [
	{otherMailbox: 'hex'},
	{newValue: 'hex'}
    ],
    output: [
	{err: 'any'},
    ]
});

methods.getFirstPacketStartingFrom = new MethodSchema({
    input: [
	{diaryNumber: 'hex'},
	{lightWeight: Boolean},
    ],
    output: [
	{err: 'any'},
	{packet: 'any'}
    ]
});

methods.handleIncomingPacket = new MethodSchema({
    input: [
	{packet: 'any'}
    ],
    output: [
	{err: 'any'}
    ]
});

methods.isAdmissible = new MethodSchema({
    input: [
	{src: 'hex'},
	{dst: 'hex'},
	{seqNumber: 'hex'}
    ],
    output: [
	{err: 'any'},
	{p: Boolean}
    ]
});

methods.getForeignDiaryNumber = new MethodSchema({
    input: [
	{otherMailbox: 'hex'}
    ],
    output: [
	{err: 'any'},
	{diaryNumber: 'hex'}
    ]
});

methods.getForeignStartNumber = new MethodSchema({
    input: [
	{otherMailbox: 'hex'}
    ],
    output: [
	{err: 'any'},
	{diaryNumber: 'hex'}
    ]
});

methods.getMailboxName = new MethodSchema({
    input: [],
    output: [
	{mailboxName: 'hex'}
    ]
});

methods.reset = new MethodSchema({
    input: [],
    output: [
	{err: 'any'}
    ]
});

methods.sendPacket = new MethodSchema({
    input: [
	{dst: 'hex'},
	{label: Number},
	{data: 'buffer'}
    ],
    output: [
	{err: 'any'}
    ]
});

methods.getTotalPacketCount = new MethodSchema({
    input: [],
    output: [
	{err: 'any'},
	{count: Number}
    ]
});








/*

  THE MAILBOX SCHEMA

 */
function MailboxSchema(methods) {
    for (methodName in methods) {
	assert(methods[methodName] instanceof MethodSchema);
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

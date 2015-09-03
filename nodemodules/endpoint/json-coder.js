var schema = require('./schemautils.js');
var mangler = require('mangler');
var assert = require('assert');
var bigint = require('./bigint.js');

// Functions to code and decode arguments according to the endpoint schema.
// Used on the web server to decode a request and encode a response.
// Used on the client to encode a request and decode a response.

function encode(argSpec, data) {
    if (argSpec == undefined) {
	assert.equal(data, undefined);
	return undefined;
    } else {
	var type = schema.getArgType(argSpec);
	if (type == 'any' || type == 'buffer') {
	    return mangler.mangle(data);
	}
	return data;
    }
}

// Takes an array of arguments passed to a function
// and encodes these arguments as a JSON object with
// keys according to argSpecs, suitable to transfer
// as a POST request.
function encodeArgs(argSpecs, args, trimmed) {
    if (trimmed) { // <-- Remove excessive part of args if it just contains undefined values.
	var len = argSpecs.length;
	var remaining = args.slice(len);
	for (var i = 0; i < remaining.length; i++) {
	    assert.equal(remaining[i], undefined);
	}
	return encodeArgs(argSpecs, args.slice(0, len));
    } else {
	assert.equal(argSpecs.length, args.length);
	var dst = {};
	for (var i = 0; i < argSpecs.length; i++) {
	    var argSpec = argSpecs[i];
	    dst[schema.getArgName(argSpec)]
		= encode(argSpec, args[i]);
	}
	return dst;
    }
}

// When passing strings as GET arguments,
// just allow strings composed of common
// characters that don't interfere with
// the syntax of GET strings.
function isValidGetString(x) {
    return !!x.match(/^[\w]*$/)

    // Maybe more specific: return !!x.match(/^[0-9a-zA-Z]*$/)
}

function encodeGetArg(argSpec, arg) {
    var type = schema.getArgType(argSpec);
    if (type == Boolean) {
	return (arg? '1' : '0');
    } else if (type == Number) {
	return '' + arg;
    } else if (type == String) {
	assert(isValidGetString(arg));
	return arg;
    } else {
	assert.equal(type, 'hex', 'Error in your schema: The type ' + type +
                     ' cannot be used with arguments passed using GET');
	assert(bigint.isBigInt(arg));
	return arg;
    }
}

function decodeGetArg(argSpec, arg) {
    var type = schema.getArgType(argSpec);
    if (type == Boolean) {
	return (arg == '1'? true : false);
    } else if (type == Number) {
	return Number.parseFloat(arg);
    } else if (type == String) {
	return arg;
    } else {
	assert.equal(type, 'hex');
	return arg;
    }
}

function encodeGetArgs(argSpecs, args) {
    assert.equal(argSpecs.length, args.length);
    var dst = new Array(args.length);
    for (var i = 0; i < args.length; i++) {
	dst[i] = encodeGetArg(argSpecs[i], args[i]);
    }
    return dst.join("/");
}

function decode(argSpec, data) {
    if (argSpec == undefined) {
	assert.equal(data, undefined);
	return undefined;
    } else {
	var type = schema.getArgType(argSpec);
	if (type == 'any' || type == 'buffer') {
	    return mangler.demangle(data);
	}
	return data;
    }
}

function decodeArgsSub(argSpecs, data, decoderFunction) {
    var dst = new Array(argSpecs.length);
    for (var i = 0; i < argSpecs.length; i++) {
	var argSpec = argSpecs[i];
	var argName = schema.getArgName(argSpec);
	dst[i] = decoderFunction(argSpec, data[argName]);
    }
    return dst;
}

function decodeArgs(argSpecs, data) {
    return decodeArgsSub(argSpecs, data, decode);
}

function decodeGetArgs(argSpecs, data) {
    return decodeArgsSub(argSpecs, data, decodeGetArg);
}

function makeGetArgPattern(argSpecs) {
    var dst = '';
    for (var i = 0; i < argSpecs.length; i++) {
	dst = dst + '/:' + schema.getArgName(argSpecs[i]);
    }
    return dst;
}


module.exports.encode = encode;
module.exports.decode = decode;
module.exports.encodeArgs = encodeArgs;
module.exports.decodeArgs = decodeArgs;
module.exports.encodeGetArgs = encodeGetArgs;
module.exports.makeGetArgPattern = makeGetArgPattern;
module.exports.decodeGetArgs = decodeGetArgs;

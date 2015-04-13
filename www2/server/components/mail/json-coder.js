var schema = require('./mailbox-schema.js');
var mangler = require('../mangler/mangler.js');
var assert = require('assert');

// Functions to code and decode arguments according to the mailbox schema.
// Used on the web server to decode a request and encode a response.
// Used on the client to encode a request and decode a response.

function encode(argSpec, data) {
    var type = schema.getArgType(argSpec);
    if (type == 'any' || type == 'buffer') {
	return mangler.mangle(data);
    }
    return data;
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
	    assert(remaining[i] == undefined);
	}
	return encodeArgs(argSpecs, args.slice(0, len));
    } else {
	assert(argSpecs.length == args.length);
	var dst = {};
	for (var i = 0; i < argSpecs.length; i++) {
	    var argSpec = argSpecs[i];
	    dst[schema.getArgName(argSpec)]
		= encode(argSpec, args[i]);
	}
	return dst;
    }
}

function decode(argSpec, data) {
    var type = schema.getArgType(argSpec);
    if (type == 'any' || type == 'buffer') {
	return mangler.demangle(data);
    }
    return data;
}

function decodeArgs(argSpecs, data) {
    var dst = new Array(argSpecs.length);
    for (var i = 0; i < argSpecs.length; i++) {
	var argSpec = argSpecs[i];
	var argName = schema.getArgName(argSpec);
	dst[i] = decode(argSpec, data[argName]);
    }
    return dst;
}

module.exports.encodeArgs = encodeArgs;
module.exports.decodeArgs = decodeArgs;

var schema = require('./mailbox-schema.js');
var JSONB = require('json-buffer');

function encode(argSpec, data) {
    var type = schema.getArgType(argSpec);
    if (type == 'any' || type == 'buffer') {
	return JSONB.stringify(data);
    }
    return data;
}

// Takes an array of arguments passed to a function
// and encodes these arguments as a JSON object with
// keys according to argSpecs, suitable to transfer
// as a POST request.
function encodeArgs(argSpecs, args) {
    assert(argSpecs.length == args.length);
    var dst = {};
    for (var i = 0; i < argSpecs.length; i++) {
	var argSpec = argSpecs[i];
	dst[schema.getArgName(argSpec)]
	    = encode(argSpec, args[i]);
    }
    return dst;
}

function decode(argSpec, data) {
    var type = schema.getArgType(argSpec);
    if (type == 'any' || type == 'buffer') {
	return JSONB.parse(data);
    }
    return data;
}

function decodeArgs(argSpecs, data) {
    var dst = new Array(argSpecs.length);
    for (var i = 0; i < argSpecs.length; i++) {
	var argSpec = argSpecs[i];
	dst[i] = decode(data[schema.getArgName(argSpec)]);
    }
    return dst;
}

module.exports.encodeArgs = encodeArgs;
module.exports.decodeArgs = decodeArgs;

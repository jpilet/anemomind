var assert = require('assert');
var request = require('request');
var JSONB = require('json-buffer');

function debugcb(err, response, body) {
    console.log('err = %j', err);
    console.log('response = %j', response);
    console.log('body = %j', body);
}

// An object that represents a connection to a server.
function ServerConnection(address, token) {
    this.address = address;
    this.authUrl = address + '/auth/local';
    this.mailRpcUrl = address + '/api/mailrpc';
    this.token = token;
}

// Method to login
ServerConnection.prototype.login = function(userdata, cb) {
    assert(userdata.email);
    assert(userdata.password);
    var self = this;
    
    var opts = {
	url: this.authUrl,
	method: 'POST',
	json: userdata
    };

    request(
	opts,
	function(err, response, body) {
	    if (err) {
		cb(err);
	    } else {
		if (response.statusCode == 200) {
		    cb(
			undefined,
			new ServerConnection(self.address, body.token)
		    );
		} else {
		    cb(
			undefined, this
		    );
		}
	    }
	}
    );
}

function makeRpcCall(self, fn) {
    return function() {
	var n = Object.keys(arguments).length;

	// Omit the callback.
	var args = new Array(n-1);
	for (var i = 0; i < n - 1; i++) {
	    args[i] = arguments[i];
	}

	// This is the callback
	var cb = arguments[arguments.length-1];

	var opts = {
	    url: self.mailRpcUrl,
	    method: 'POST',

	    // Stringify it manually using JSONB, in order to
	    // get the buffers right.
	    json: {
		fn: fn,
		args: JSONB.stringify(args)
	    }
	};

	// Call it
	request(opts, function(err, response, body) {
	    if (err) {
		cb(err);
	    } else {
		cb(JSONB.parse(body.err), JSONB.parse(body.result));
	    }
	});
    }
}

// To register RPC calls
ServerConnection.prototype.registerCalls = function(functions) {
    var self = this;
    for (var i = 0; i < functions.length; i++) {
	var fn = functions[i];
	assert(typeof fn == 'string');
	this[fn] = makeRpcCall(self, fn);
	assert(this[fn] != undefined);
    }
}

module.exports = ServerConnection;

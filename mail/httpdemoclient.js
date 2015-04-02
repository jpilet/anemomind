// A demo client for the HTTP api

var assert = require('assert');
var request = require('request');
var JSONB = require('json-buffer');

function Server(address, token) {
    this.address = address;
    this.authUrl = address + '/auth/local';
    this.mailRpcUrl = address + '/api/mailrpc';
    this.token = token;
}

// curl -d "{\"email\":\"kalle@abc.com\", \"password\":\"abc\"}" -H "Content-type: application/json" http://localhost:9000/auth/local

// Test user that we use:
//
var testuser = {
    'email': 'kalle@abc.com',
    'password': 'abc'
};

function debugcb(err, response, body) {
    console.log('err = %j', err);
    console.log('response = %j', response);
    console.log('body = %j', body);
}


Server.prototype.login = function(userdata, cb) {
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
			new Server(self.address, body.token)
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

Server.prototype.registerCalls = function(functions) {
    for (var i = 0; i < functions.length; i++) {
	var fn = functions[i];
	assert(typeof fn == 'string');
	this[fn] = function() {
	    var n = Object.keys(arguments).length;

	    // Omit the callback.
	    var args = new Array(n-1);
	    for (var i = 0; i < n - 1; i++) {
		args[i] = arguments[i];
	    }

	    // This is the callback
	    var cb = arguments[arguments.length-1];

	    var opts = {
		url: this.mailRpcUrl,
		method: 'POST',

		// Stringify it manually using JSONB, in order to
		// get the buffers right.
		json: {
		    fn: fn,
		    args: JSONB.stringify(args)
		}
	    };

	    // Call it
	    request(opts, cb);
	    return opts;
	}
	assert(this[fn] != undefined);
    }
}








// Always have 'http://' at the beginning.
var address = 'http://localhost:9000';
var s = new Server(address);
s.login(testuser, function(err, server) {

    server.registerCalls(['add']);
    console.log('err = %j', err);
    console.log('server = %j', server);
    console.log(server.add(3, 4, 5, debugcb));
    
});


// A demo client for the HTTP api

var assert = require('assert');
var request = require('request');
var JSONB = require('json-buffer');

function Server(address, token) {
    this.address = address;
    this.authurl = address + '/auth/local';
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

Server.prototype.registerCalls = function(calls) {
    for (var i = 0; i < calls.length; i++) {
	var call = calls[i];
	assert(typeof call == 'string');
	this[call] = function() {
	    var rpcdata = [call].concat(arguments.slice(0, arguments.length-1));
	    var cb = arguments[arguments.length-1];
	    
	    var opts = {
		headers : {
		    'url': this.mailRpcUrl,
		    'method': 'POST',
		    
		    // We don't want express to automatically decode it
		    // for us on the server side: The hander should decode
		    // it using the json-buffer library.
		    'Content-type': 'text/plain' 
		},
		body: JSONB.stringify(rpcdata)
	    };

	    // Call it
	    request(opts, cb);
	}
    }
}








// Always have 'http://' at the beginning.
var address = 'http://localhost:9000';
(new Server(address)).login(testuser, function(err, server) {

    console.log('err = %j', err);
    console.log('server = %j', server);
    //server.registerCalls(['add']);
    
});


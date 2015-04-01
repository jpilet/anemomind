// A demo client for the HTTP api

var assert = require('assert');
var request = require('request');

function Server(address) {
    this.address = address;
    this.authurl = address + '/auth/local';
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
    var opts = {
	url: this.authurl,
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
		    cb(undefined, {
			success: true,
			token: body.token
		    });
		} else {
		    cb(undefined, {success: false});
		}
	    }
	}
    );
}



// Always have 'http://' at the beginning.
var address = 'http://localhost:9000';

var server = new Server(address);

server.login(testuser, function(err, data) {
    console.log('data = %j', data);
});


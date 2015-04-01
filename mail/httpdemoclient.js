// A demo client for the HTTP api

var assert = require('assert');
var request = require('request');

function Server(address, token) {
    this.address = address;
    this.authurl = address + '/auth/local';
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








// Always have 'http://' at the beginning.
var address = 'http://localhost:9000';
(new Server(address)).login(testuser, function(err, server) {
    
    //
    
});


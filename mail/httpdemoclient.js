// A demo client for the HTTP api

var request = require('request');

// Always have 'http://' at the beginning.
var address = 'http://localhost:9000';
var authurl = address + '/auth/local';

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


function login(cb) {
    var opts = {
	url: authurl,
	method: 'POST',
	json: testuser
    };

    request(
	opts,
	debugcb
    );
}

function test() {
    request(address, debugcb);
}

login();

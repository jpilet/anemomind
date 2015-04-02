// A demo client for the HTTP api

var ServerConnection = require('./server-connection.js');

// curl -d "{\"email\":\"kalle@abc.com\", \"password\":\"abc\"}" -H "Content-type: application/json" http://localhost:9000/auth/local

// Test user that we use:
//
var testuser = {
    'email': 'kalle@abc.com',
    'password': 'abc'
};



function addNumbersDemo() {
    // Always have 'http://' at the beginning.
    var address = 'http://localhost:9000';
    var s = new ServerConnection(address);
    s.login(testuser, function(err, server) {
	s.registerCalls(['add']);
	s.add(3, 4, 5, function (err, result) {
	    console.log('Got error: %j', err);
	    console.log('Got result: %j', result);
	});
	
    });
}


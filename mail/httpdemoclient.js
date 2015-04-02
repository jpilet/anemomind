// A demo client for the HTTP api

var ServerConnection = require('./server-connection.js');
var mb = require('./mail.http.js');

// curl -d "{\"email\":\"kalle@abc.com\", \"password\":\"abc\"}" -H "Content-type: application/json" http://localhost:9000/auth/local

// Test user that we use:
//
var testuser = {
    'email': 'kalle@abc.com',
    'password': 'abc'
};

var address = 'http://localhost:9000';


function addNumbersDemo() {
    // Always have 'http://' at the beginning.
    var s = new ServerConnection(address);
    s.login(testuser, function(err, server) {
	s.registerCalls(['add']);
	s.add(3, 4, 5, function (err, result) {
	    console.log('Got error: %j', err);
	    console.log('Got result: %j', result);
	});
	
    });
}

function mailboxDemo() {

    mb.tryMakeMailbox(
	address, testuser, 'a',
	function (err, mailbox) {
	    mailbox.getMailboxName(function(err, mailboxName) {
		console.log('Error: %j', err);
		console.log('The mailbox name is: %j', mailboxName);
	    });
	});    
}

//mailboxDemo();
addNumbersDemo();

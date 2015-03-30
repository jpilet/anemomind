var btserver = require('./btserver.js');
var mb = require('mail.sqlite.js');

var mailbox = new mb.Mailbox(
    ':memory:',
    'a',
    function(err) {
	if (err) {
	    console.log('There was an error');
	} else {
	    btserver.runMailService('a', mailbox);
	}
    }
);

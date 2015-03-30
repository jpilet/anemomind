var btserver = require('./btserver.js');
var mb = require('./mail.sqlite.js');
var bigint = require('./bigint.js');

console.log('Make a new mailbox!');
var mailbox = new mb.Mailbox(
    ':memory:',
    bigint.make(0),
    function(err) {
	if (err) {
	    console.log('There was an error');
	} else {
	    console.log('Run the mailservice');
	    btserver.runMailService('anemobox', mailbox);
	}
    }
);

var btserver = require('./btserver.js');
var mb = require('./mail.sqlite.js');
var bigint = require('./bigint.js');

var mailbox = new mb.Mailbox(
    ':memory:',
    bigint.make(0),
    function(err) {
	if (err) {
	    console.log('There was an error');
	} else {
	    btserver.runMailService('anemobox', mailbox);
	}
    }
);

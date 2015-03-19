var mailsqlite = require('../mail.sqlite.js');
var assert = require('assert');

var withbox = function(cb) {
    var box = new mailsqlite.Mailbox(
	':memory:', 'demobox',
	function(err) {
	    cb(box);
	});
};

describe('Send acknowledge',
	 function() {
	     
	     it('Should make sure that packet data is registered',
	       function() {
		   
	       });


	     
	     it('Should send 30 packets and make sure that there are 31 packets (one ack produced',
		function() {

		    var box = new mailsqlite.Mailbox(
			':memory:',
			'demobox',
			function(err) {
			    // This is a function that sends 30 packets, then call cb
			    var spammer = function(n, cb) {
				if (n == 0) {
				    cb();
				} else {
				    box.handleIncomingPacket(
					new pkt.Packet(
					    'some-spammer', box.mailboxName,
					    n, -1, 'Spam message',
					    'There are ' + n + ' messages left to send'),
					function(err) {
					    spammer(n - 1, cb);
					});
				}
			    };

			    // Call the spammer here
			    spammer(30, function(err) {
				assert(err == undefined);
				var query = 'SELECT * FROM packets WHERE src = ?';
				box.db.get(
				    query, box.mailboxName,
				    function(err, results) {
					assert.equal(results.length, 31);
					done();
				    });
			    });
			}
		    );

		    
		    
		});});



var mailsqlite = require('../mail.sqlite.js');
var assert = require('assert');

var withbox = function(cb) {
    var box = new mailsqlite.Mailbox(
	':memory:', 'demobox',
	function(err) {
	    cb(box);
	});
};

describe(
    'Failing test',
    function() {
	it('SHOULD FAIL', function() {
	    assert.equal(-1, 0);
	});
    });
	 

describe(
    'Send acknowledge',
    function() {
	it('Should send 30 packets and make sure that there are 31 packets (one ack produced)',
	   function() {
	       //console.log('REACHES HERE 1');
	       var box = new mailsqlite.Mailbox(
		   ':memory:', 'demobox',
		   function(err) {
		       console.log('REACHES HERE 2');
		       assert.equal(0, 1);

		       
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
				   console.length('RESULTS LENGTH = ', results.length);
				   assert.equal(results.length, 29);
				   done();
			       });
		       });
		   }
	       );
	       

	       
	       
	   });});



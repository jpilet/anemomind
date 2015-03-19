var mailsqlite = require('../mail.sqlite.js');
var pkt = require('../packet.js');
var assert = require('assert');


var withbox = function(cb) {
    var box = new mailsqlite.Mailbox(
	':memory:', 'demobox',
	function(err) {
	    cb(box);
	});
};

// describe(
//     'Failing test',
//     function() {
// 	it('SHOULD FAIL', function() {
// 	    assert.equal(-1, 0);
// 	});
//     });
	 

describe(
    'Send acknowledge',
    function() {
	it('Should send packets so that an ack is produced',
	   function(done) {
	       var box = new mailsqlite.Mailbox(
		   ':memory:', 'demobox',
		   function(err) {
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
		       spammer(box.ackFrequency, function(err) {
			   assert(err == undefined);
			   var query = 'SELECT * FROM packets WHERE src = ?';
			   box.db.all(
			       query, box.mailboxName,
			       function(err, results) {
				   var r = results[0];
				   assert.equal(results.length, 1);
				   assert.equal(r.label, 'ack');
				   assert.equal(r.src, 'demobox');
				   assert.equal(r.dst, 'some-spammer');
				   var query = 'SELECT * FROM packets WHERE dst = ?';
				   box.db.all(
				       query, box.mailboxName,
				       function(err, results) {
					   assert.equal(results.length, box.ackFrequency);
					   done();
				       });
			       });
		       });
		   }
	       );
	       

	       
	       
	   });});



var mailsqlite = require('../mail.sqlite.js');
var pkt = require('../packet.js');
var assert = require('assert');
var intarray = require('../intarray.js');


var withbox = function(cb) {
    var box = new mailsqlite.Mailbox(
	':memory:', 'demobox',
	function(err) {
	    assert(err == undefined);
	    cb(box);
	});
};



/* // Template that you can reuse when building new test cases:

   
describe(
    'name of code to test',
    function() {
	it(
	    'Do something',
	    function(done) {
		withbox(
		    function(box) {

			
			// Insert code here.
			assert(true);
			done();

			
		    }
		);
	    }
	);
    }
);


*/







describe(
    'getOrMakeCNumber',
    function() {
	it(
	    'get or make a C number',
	    function(done) {
		withbox(
		    function(box) {
			box.getOrMakeCNumber('abra', 12349, function(err, cnumber) {
			    assert(cnumber == 12349);
			    box.getOrMakeCNumber('abra', 19999, function(err, cnumber) {
				// unchanged, because there is already a number there.
				assert(cnumber == 12349);
				done();
			    });
			});
		    }
		);
	    }
	);
    }
);








describe(
    'makeNewDiaryNumber',
    function() {
	it(
	    'Generate a new diary number',
	    function(done) {
		withbox(
		    function(box) {
			box.getLastDiaryNumber(function(err, num) {
			    assert(num == undefined);
			    box.makeNewDiaryNumber(function(err, num) {
				assert(typeof num == 'number');
				done();
			    });
			});
		    }
		);
	    }
	);
    }
);





describe(
    'getLastDiaryNumber',
    function() {
	it(
	    'Retrieve the last diary number',
	    function(done) {
		withbox(
		    function(box) {
			box.db.run(
			    'INSERT INTO packets VALUES (?, ?, ?, ?, ?, ?, ?, ?)',
    			    129, "abra", "kadabra", 119, 109,
			    "testpacket", "sometestdata", false,
    			    function(err) {
    				box.getLastDiaryNumber(function(err, num) {
				    assert(num == 129);
				    done();
    				});
    			    }
			);
		    }
		);
	    }
	);
    }
);


describe(
    'makeNewSeqNumber',
    function() {
	it(
	    'Generate unique sequence numbers for a destination mailbox in a sequence',
	    function(done) {
		withbox(
		    function(box) {
			box.makeNewSeqNumber('abra', function(err, x) {
			    box.makeNewSeqNumber('abra', function(err, y) {
				assert(x + 1 == y);
				box.makeNewSeqNumber('abra', function(err, z) {
				    assert(y + 1 == z);
				    done();
				});
			    });
			});
		    }
		);
	    }
	);
    }
);

describe(
    '{set, get}ForeignDiaryNumber',
    function() {
	it(
	    'Set and get a foreign diary number',
	    function(done) {
		withbox(
		    function(box) {
			box.getForeignDiaryNumber('rulle', function(err, value) {
			    assert(value == undefined);
			    box.setForeignDiaryNumber('rulle', 119, function(err) {
				box.getForeignDiaryNumber('rulle', function(err, value2) {
				    assert(value2 == 119);
				    box.setForeignDiaryNumber('rulle', 135, function(err) {
					box.getForeignDiaryNumber('rulle', function(err, value3) {
					    assert(value3 == 135);
					    done();
					});
				    });
				});
			    });
			});
		    }
		);
	    }
	);
    }
);




describe(
    'getFirstPacketStartingFrom',
    function() {
	it(
	    'Retrieve the first packet starting from a diary number',
	    function(done) {
		withbox(
		    function(box) {
			box.getFirstPacketStartingFrom(0, function(err, result) {
			    assert(result == undefined);
			    box.sendPacket('dst', 'label', new Buffer(1), function(err) {
				box.getFirstPacketStartingFrom(0, function(err, result) {
				    assert(result != undefined);
				    box.getFirstPacketStartingFrom(
					result.diarynumber + 1, function(err, result) {
					    assert(result == undefined);
					    done();
					});
				});
			    });
			});
		    }
		);
	    }
	);
    }
);






describe(
    'updateCTable',
    function() {
	it(
	    'Update the C table appropriately',
	    function(done) {
		withbox(
		    function(box) {

			box.updateCTable('a', 'b', 19, function(err) {
			    box.getCNumber('a', 'b', function(err, value) {
				assert(value == 19);
				box.updateCTable('a', 'b', 29, function(err) {
				    box.getCNumber('a', 'b', function(err, value) {
					assert(value == 29);
					box.updateCTable('a', 'b', 13, function(err) {
					    box.getCNumber('a', 'b', function(err, value) {
						assert(value == 29);
						done();
					    });
					});
				    });
				});
			    });
			});
		    }
		);
	    }
	);
    }
);






describe(
    'registerPacketData',
    function() {
	it(
	    'Should store a packet in the DB',
	    function(done) {
		withbox(
		    function(box) {
			box.registerPacketData(
			    new pkt.Packet(
				'a', 'b', 119, 30, 'My label',
				'Some data'), function(err) {
				assert(err == undefined);
				box.getCNumber('a', 'b', function(err, num) {
				    assert(err == undefined);
				    assert(num == 30);
				    box.getLastDiaryNumber(function(err, dnum) {
					assert(err == undefined);
					box.getFirstPacketStartingFrom(
					    dnum, function(err, packet) {
						
						assert(packet.src == 'a');
						assert(packet.dst == 'b');
						assert(packet.seqnumber == 119);
						assert(packet.cnumber == 30);
						assert(packet.label == 'My label');
						assert(packet.data == 'Some data');

						done();
					});
				    });
				});
			    });
		    }
		);
	    }
	);
    }
);
	 
describe(
    'handleIncomingPacket',
    function() {
	it('Should send packets so that an ack packet is produced to the spammer',
	   function(done) {
	       withbox(function(box) {
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
				   var nums = intarray.deserialize(r.data);
				   assert.equal(nums.length, box.ackFrequency);
				   for (var i = 0; i < nums.length; i++) {
				       assert(1 <= nums[i]);
				       assert(nums[i] <= box.ackFrequency);
				   }
				   var query = 'SELECT * FROM packets WHERE dst = ?';
				   box.db.all(
				       query, box.mailboxName,
				       function(err, results) {
					   assert.equal(results.length, box.ackFrequency);

					   // When an ack is sent, the packets have
					   // their 'ack' flags set to true.
					   for (var i = 0; i < results.length; i++) {
					       assert(results[i].ack);
					   }
					   
					   done();
				       });
			       });
		       });
		   }
	       );
	   });});





function fillPackets(box, ackFn, n, cb) {
    if (n == 0) {
	cb();
    } else {
	var query = 'INSERT INTO packets VALUES (?, ?, ?, ?, ?, ?, ?, ?)';
	box.db.run(
	    query, n + 119,
	    box.mailboxName, 'destination', n,
	    0, 'some label', new Buffer(1),
	    ackFn(n),
	    function (err) {
		assert(err == undefined);
		fillPackets(box, ackFn, n-1, cb);
	    }
	);
    }
}

function maximizeAndGetCNumber(box, cb) {
    box.maximizeCNumber(
	'destination',
	function(err) {
	    assert(err == undefined);
	    box.getCNumber(
		box.mailboxName,
		'destination',
		cb
	    );
	}
    );
}


describe(
    'maximizeCNumber',
    function() {
	it(
	    'Should maximize the C number to 15, and remove the obsolete packets.',
	    function(done) {
		withbox(
		    function(box) {
			fillPackets(
			    box,
			    function(i) {
				return i < 15 || i == 20;
			    },
			    30,
			    function(err) {
				assert(err == undefined);
				maximizeAndGetCNumber(
				    box,
				    function(err, value) {
					assert(err == undefined);
					assert(value == 15);
					box.getTotalPacketCount(
					    function(err, value) { // 1--14 removed => 16 remain
						assert(value == 16);
						done();
					    }
					);
				    }
				);
			    }
			);
		    }
		);
	    }
	);




	it(
	    'Should maximize the C number to 31',
	    function(done) {
		withbox(
		    function(box) {
			fillPackets(
			    box,
			    function(i) {
				return true;
			    },
			    30,
			    function(err) {
				assert(err == undefined);
				maximizeAndGetCNumber(
				    box,
				    function(err, value) {
					assert(err == undefined);
					assert(value == 31);
					done();
				    }
				);
			    }
			);
		    }
		);
	    }
	);






	it(
	    'C-number should remain undefined',
	    function(done) {
		withbox(
		    function(box) {
			maximizeAndGetCNumber(
			    box,
			    function(err, value) {
				assert(err == undefined);
				assert(value == undefined);
				done();
			    }
			);
		    }
		);
	    }
	);
    }
);

describe(
    'sendPacket',
    function() {
	it(
	    'Send two packets, and unique diary numbers',
	    function(done) {
		withbox(
		    function(box) {

			
			box.sendPacket(
			    'dst-name',
			    'some-label',
			    new Buffer(1),
			    function() {
				box.sendPacket(
				    'dst-name',
				    'some-label',
				    new Buffer(1),
				    function() {
					box.db.all(
					    'SELECT diarynumber FROM packets',
					    function (err, results) {
						assert(err == undefined);
						assert(results.length == 2);
						assert(
						    Math.abs(
							results[0].diarynumber
							    - results[1].diarynumber
						    ) == 1
						);
						done();
					    }
					);
				    }
				);
			    }
			);
		    }
		);
	    }
	);
    }
);

function fillWithPackets(count, srcMailbox, dstMailboxName, cb) {
    assert(typeof count == 'number');
    assert(typeof dstMailboxName == 'string');
    if (count == 0) {
	cb();
    } else {
	srcMailbox.sendPacket(
	    dstMailboxName,
	    "Some-label" + count,
	    new Buffer(3),
	    fillWithPackets(count-1, srcMailbox, dstMailboxName, cb)
	);
    }
}


describe(
    'sendPacket',
    function() {
	it(
	    'Send many packets',
	    function(done) {
		withbox(
		    function(box) {

			
			fillWithPackets(39, box, 'B', function(err) {
			    assert(err == undefined);
			    done();
			});
			
		    }
		);
	    }
	);
    }
);


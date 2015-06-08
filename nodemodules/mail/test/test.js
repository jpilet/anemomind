var mailsqlite = require('../mail.sqlite.js');
var assert = require('assert');
var bigint = require('../bigint.js');
var schema = require('../mailbox-schema.js');
var common = require('../common.js');



var withbox = function(cb) {
  mailsqlite.tryMakeMailbox(
    ':memory:', 'aaabbb',
    function(err, box) {
      assert.equal(err, undefined);
      cb(box);
    }
  );
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
	    box.getOrMakeCNumber(box.db, 'abc', '12349', function(err, cNumber) {
	      assert.equal(err, undefined);
	      assert.equal(cNumber, '12349');
	      box.getOrMakeCNumber(box.db, 'abc', '19999', function(err, cNumber) {
		// unchanged, because there is already a number there.
		assert.equal(cNumber, '12349');
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
	    box.getLastDiaryNumber(box.db, function(err, num) {
	      assert.equal(num, undefined);
	      box.makeNewDiaryNumber(box.db, function(err, num) {
		assert(common.isCounter(num));
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
    	      '129', 'a', 'c', '119', '109',
	      0, 'sometestdata', false,
    	      function(err) {
    		box.getLastDiaryNumber(box.db, function(err, num) {
		  assert.equal(num, '129');
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
	    box.makeNewSeqNumber(box.db, 'aa', function(err, x) {
	      box.makeNewSeqNumber(box.db, 'aa', function(err, y) {
		assert.equal(bigint.inc(x), y);
		box.makeNewSeqNumber(box.db, 'aa', function(err, z) {
		  assert.equal(bigint.inc(y), z);
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
	    box.getForeignDiaryNumber('eff', function(err, value) {
	      assert.equal(value, undefined);
	      box.setForeignDiaryNumber('eff', '119', function(err) {
		box.getForeignDiaryNumber('eff', function(err, value2) {
		  assert.equal(value2, '119');
		  box.setForeignDiaryNumber('eff', '135', function(err) {
		    box.getForeignDiaryNumber('eff', function(err, value3) {
		      assert.equal(value3, '135');
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
	    box.getFirstPacketStartingFrom('0', false, function(err, result) {
	      assert.equal(result, undefined);
	      box.sendPacket('ddd', 49, new Buffer(1), function(err) {
		box.getFirstPacketStartingFrom('0', false, function(err, result) {
		  assert.notEqual(result, undefined);
		  box.getFirstPacketStartingFrom(
		    bigint.inc(result.diaryNumber),
		    false, function(err, result) {
		      assert.equal(result, undefined);
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

	    box.updateCTable(box.db, 'a', 'b', '19', function(err) {
	      box.getCNumber(box.db, 'a', 'b', function(err, value) {
		assert.equal(value, '19');
		box.updateCTable(box.db, 'a', 'b', '29', function(err) {
		  box.getCNumber(box.db, 'a', 'b', function(err, value) {
		    assert.equal(value, '29');
		    box.updateCTable(box.db, 'a', 'b', '13', function(err) {
		      box.getCNumber(box.db, 'a', 'b', function(err, value) {
			assert.equal(value, '29');
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
	    box.registerPacketData(box.db,
	      {src: 'a', dst: 'b', seqNumber: '119', cNumber: '030',
	       label: 49, data: mailsqlite.serializeString('Some data')},
	      function(err) {
		assert.equal(err, undefined);
		box.getCNumber(box.db, 'a', 'b', function(err, num) {
		  assert.equal(err, undefined);
		  assert.equal(num, '030');
		  box.getLastDiaryNumber(box.db, function(err, dnum) {
		    assert.equal(err, undefined);
		    box.getFirstPacketStartingFrom(
		      dnum, false, function(err, packet) {
			
			assert.equal(packet.src, 'a');
			assert.equal(packet.dst, 'b');
			assert.equal(packet.seqNumber, '119');
			assert.equal(packet.cNumber, '030');
			assert.equal(packet.label, 49);
			assert.equal(packet.data, 'Some data');

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
		 {
		   src: 'caa',
		   dst: box.mailboxName,
		   seqNumber: bigint.make(n),
		   cNumber: bigint.make(0),
		   label: 49,
		   data: mailsqlite.serializeString('Some spam message'),
		 },
		 function(err) {
		   spammer(n - 1, cb);
		 });
	     }
	   };

	   // Call the spammer here
	   spammer(box.ackFrequency, function(err) {
	     assert.equal(err, undefined);
	     var query = 'SELECT * FROM packets WHERE src = ?';
	     box.db.all(
	       query, box.mailboxName,
	       function(err, results) {
		 var r = results[0];
		 assert.equal(results.length, 1);
		 assert.equal(r.label, common.ack);
		 assert.equal(r.src, 'aaabbb');
		 assert.equal(r.dst, 'caa');
		 var nums = mailsqlite.deserializeSeqNums(r.data);
		 assert.equal(nums.length, box.ackFrequency);
		 for (var i = 0; i < nums.length; i++) {
		   assert(bigint.make(1) <= nums[i]);
		   assert(nums[i] <= bigint.make(box.ackFrequency));
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
      query, bigint.make(n + 119),
      box.mailboxName, 'ddd', bigint.make(n),
      bigint.make(0), 49, new Buffer(1),
      ackFn(n),
      function (err) {
	assert.equal(err, undefined);
	fillPackets(box, ackFn, n-1, cb);
      }
    );
  }
}

function maximizeAndGetCNumber(box, cb) {
  box.maximizeCNumber(box.db,
    'ddd',
    function(err) {
      assert.equal(err, undefined);
      box.getCNumber(box.db, 
	box.mailboxName,
	'ddd',
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
		assert.equal(err, undefined);
		maximizeAndGetCNumber(
		  box,
		  function(err, value) {
		    assert.equal(err, undefined);
		    assert.equal(value, bigint.make(15));
		    box.getTotalPacketCount(
		      function(err, value) { // 1--14 removed => 16 remain
			assert.equal(value, 16);
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
		assert.equal(err, undefined);
		maximizeAndGetCNumber(
		  box,
		  function(err, value) {
		    assert.equal(err, undefined);
		    
		    assert.equal(value, bigint.make(31));
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
		assert.equal(err, undefined);
		assert.equal(value, undefined);
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
	      'abb',
	      49,
	      new Buffer(1),
	      function() {
		box.sendPacket(
		  'abb',
		  49,
		  new Buffer(1),
		  function() {
		    box.db.all(
		      'SELECT diaryNumber FROM packets',
		      function (err, results) {
			assert.equal(err, undefined);
			assert.equal(results.length, 2);
			var a = results[0].diaryNumber;
			var b = results[1].diaryNumber;
			assert(a == bigint.inc(b) ||
			       b == bigint.inc(a));
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
  assert.equal(typeof count, 'number');
  assert.equal(typeof dstMailboxName, 'string');
  if (count == 0) {
    cb();
  } else {
    srcMailbox.sendPacket(
      dstMailboxName,
      49 + count,
      new Buffer(3),
      function(err) {
	if (err == undefined) {
	  fillWithPackets(count-1, srcMailbox, dstMailboxName, cb)
	} else {
	  cb(err);
	}
      }
    );
  }
}

var expand = mailsqlite.expand;

function spanWidth(span) {
  if (typeof span[0] == 'number') {
    return span[1] - span[0];	
  } else {
    assert(bigint.isBigIntStrict(span[0]));
    var counter = 0;
    var x = span[0];
    while (x < span[1]) {
      x = bigint.inc(x);
      counter++;
    }
    return counter;
  }
}

describe(
  'sendPacket2',
  function() {
    it(
      'Send many packets',
      function(done) {
	withbox(
	  function(box) {

	    
	    fillWithPackets(
	      39, box, 'b',
	      function(err) {
		box.db.all(
		  'SELECT * FROM packets',
		  function (err, results) {
		    assert.equal(err, undefined);
		    assert.equal(results.length, 39);

		    var seqnumSpan = undefined;
		    var diarynumSpan = undefined;

		    for (var i = 0; i < results.length; i++) {
		      var r = results[i];
		      seqnumSpan = expand(seqnumSpan, r.seqNumber);
		      diarynumSpan = expand(diarynumSpan, r.diaryNumber);
		    }

		    assert.equal(spanWidth(seqnumSpan) + 1, 39);
		    assert.equal(spanWidth(diarynumSpan) + 1, 39);
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

describe(
  'sendPackets',
  function() {
    it(
      'Should send many packets',
      function(done) {
	withbox(
	  function(box) {

	    box.sendPackets(
	      'abc',
	      9,
	      [new Buffer(1), new Buffer(2), new Buffer(3)],
	      function (err) {
		assert.equal(err, undefined);
		box.getAllPackets(function(err, packets) {
		  assert.equal(packets.length, 3);
		  var marks = [false, false, false];
		  for (var i = 0; i < 3; i++) {
		    marks[packets[i].data.length-1] = true;
		  }
		  for (var i = 0; i < 3; i++) {
		    assert(marks[i]);
		  }
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
  'mailbox-schema',
  function() {
    it(
      'Check that the object conforms with the schema',
      function(done) {
	withbox(
	  function(box) {

	    assert(schema.isValidMailbox(box));
	    done();

	    
	  }
	);
      }
    );
  }
);




describe('Mailbox on file system', function() {
  it(
    'A mailbox on the file system',
    function(done) {
      mailsqlite.tryMakeMailbox('/tmp/mailbox.db', 'abc', function(err, mb) {
	assert.equal(err, undefined);
	assert(mb);
	assert(mb.db);
	mb.reset(function(err) {
	  assert.equal(err, undefined);
	  mb.sendPacket('rulle', 122, new Buffer(0), function(err) {
	    assert.equal(err, undefined);
	    mb.getTotalPacketCount(function(err, n) {
	      assert.equal(err, undefined);
	      assert.equal(n, 1);
	      mb.close(function(err) {
		assert.equal(err, undefined);
		done();
	      });
	    });
	  });
	});
      });
    });
});

describe('Mailbox open and close', function() {
  it(
    'Should close and open a mailbox',
    function(done) {
      mailsqlite.tryMakeMailbox('/tmp/mailbox.db', 'abc', function(err, mb) {
        assert(!err);
        assert(mb.db);
        mb.close(function(err) {
          assert(!err);
          assert(!mb.db);
          mb.open(function(err) {
            assert(!err);
            assert(mb.db);
            done();
          })
        });
      });
    });
});



/*

Mailbox model based on sqlite

*/

var sqlite3 = require('sqlite3').verbose();
var seqnums = require('./seqnums.js');
var async = require('async');
var pkt = require('./packet.js');


function assert(x) {
    if (!x) {
	throw new Error('Assertion failed');
    }
}


/////////////////////////////////////////////////////////
// General functions for checking if an object is a string
function isString(x) {
    if (typeof x == 'string') {
	return true;
    }
    return false;
}

function isNonEmptyString(x) {
    if (isString(x)) {
	return 0 < x.length;
    }
    return false;
}

/////////////////////////////////////////////////////////
function isValidDBFilename(x) {
    return isNonEmptyString(x);
}

function isValidMailboxName(x) {
    return isNonEmptyString(x);
}

function runWithLog(db, cmd) {
    console.log(cmd);
    db.run(cmd);
}

// Assembles an SQL command as a string to create a table.
function makeCreateCmd(tableName, fieldSpecs) {
    var result = 'CREATE TABLE ' + tableName + ' (';
    for (var i = 0; i < fieldSpecs.length; i++) {
	result += fieldSpecs[i];
	if (i < fieldSpecs.length - 1) {
	    result += ', ';
	}
    }
    return result + ')';
}

// Checks if a table exists, and calls 'cb' with that information, or error.
function tableExists(db, tableName, cb) {
    
    // If no callback is provided, provide a default callback for debugging purposes.
    if (cb == undefined) {
	cb = function(status) {
	    console.log('DEBUG INFO:');
	    if (status == true) {
		console.log('  There is a table with name ' + tableName);
	    } else if (status == false) {
		console.log('  There is NO table with name ' + tableName);
	    } else {
		console.log('  Error while testing existence of table with name ' + tableName);
	    }
	};
    }

    // The query
    db.get('SELECT name FROM sqlite_master WHERE type=\'table\' AND name=\''
	   + tableName + '\'', function(err, row) {
	       if (err == undefined) {
		   if (row == undefined) {
		       cb(false);
		   } else {
		       cb(true);
		   }
	       } else {
		   cb(err);
	       }
	   });
}

function getAllTables(db, cb) {
    db.all('SELECT * FROM sqlite_master WHERE type=\'table\'', cb);
}

function getAllTableRows(db, tableName, cb) {
    db.all('SELECT * FROM ' + tableName, cb);
}

// For debugging. The callback cb is optional
function dispAllTableData(db, cb) {
    getAllTables(db, function(err, tables) {
	async.map(
	    tables,
	    function(table, a) {
		getAllTableRows(db, table.name, a);
	    },
	    function(err, tableData) {
		if (err == undefined) {
		    if (tables.length != tableData.length) {
			console.log('tables = %j', tables);
			console.log('tableData = %j', tableData);
			throw new Error('Array length mismatch');
		    }

		    console.log('=== TABLE SUMMARY ===');
		    for (var i = 0; i < tables.length; i++) {
			var ind = i+1;
			console.log('');
			console.log('');
			console.log('Table ' + ind + ' of ' + tables.length + ' is named "' +
				    tables[i].name
				    + '" and contains:');
			console.log('  %j', tableData[i]);
			console.log('TOTAL OF ' + tableData[i].length + ' ITEMS.');
		    }
		} else if (cb == undefined) {
		    throw new Error('There was an error');
		}

		if (cb != undefined) {
		    cb(err);
		}
	    })});
    
}


// Creates a new table if it doesn't exist already.
function initializeTableIfNotAlready(db,            // <-- A sqlite3 database
				     tableName,     // <-- Name of the table to be created.
				     fieldSpecs,    // <-- The fields of the table
				     cb) {          // <-- A callback that accepts optional err.
    tableExists(db, tableName, function(status) {
	if (status == false) {
	    db.run(makeCreateCmd(tableName, fieldSpecs), cb);
	} else if (status == true) {
	    cb();
	} else { // Pass on the error code to the callback.
	    cb(status);
	}
    });
}


// This is a common table for all packets that exist
// inside this mailbox: that includes packets sent
// from this mailbox, packets that are transferred
// via this mailbox, and packets delivered to this mailbox.
function initializePacketsTable(db, cb) {
    initializeTableIfNotAlready(
	db, 'packets',
	['diarynumber BIGINT', // <-- Every packet has a diary number. This is used to refer to the packet within this mailbox.
	 'src TEXT', // <-- The identifier of the mailbox sending the packet.
	 'dst TEXT',         // <-- The mailbox to which the packet is sent
	 'seqnumber BIGINT', // <-- Determines how old this packet is and if it can be ignored.
	 'cnumber BIGINT', // <-- An update of the last c-number, used to update the c-table.
	 'label TEXT', // <-- What this packet is about. The label 'ack' is reserved for acknowledgement packets.
	 'data BLOB', // <-- The data of this packet. To be on a form specified by a higher level protocol
	 'ack INTEGER' // <-- boolean, only used for packets originating from this mailbox.
	], cb);
}

// This is a table with the sequence number counters
// for all other mailboxes to which this mailbox sends
// packets. The value of the counter should be one more
// than that of the last packet sent.
function initializeSeqNumbersTable(db, cb) {
    initializeTableIfNotAlready(
	db, 'seqnumbers',
	['dst TEXT', // <-- The destination mailbox.
	 'counter BIGINT'], // <-- Next sequence number to be assigned a newly created packet.
	cb
    );
}

// The C-table holds a number for every
// (src, dst) pair. It will only let a packet
// through if its sequence number is at least
// that of the C-number.
function initializeCTable(db, cb) {
    initializeTableIfNotAlready(
	db, 'ctable',
	['src TEXT', // <-- source mailbox identifier
	 'dst TEXT', // <-- destination mailbox identifier
	 'counter BIGINT'], // <-- Last known maximum c-number, used to reject packets that are too old.
	cb
    );
}

// The diary number table is a table with
// diary numbers for foreign mailboxes
// with which this mailbox has synchronized.
// It is used by this mailbox whenever it synchronizes
// with another mailbox by only fetching packets
// with diary numbers greater than that in this table.
function initializeDiaryNumberTable(db, cb) {
    initializeTableIfNotAlready(
	db, 'diarynumbers',
	['mailbox TEXT',  // <-- identifier of the other mailbox
	 'number BIGINT'], // <-- The diary number
	cb
    );
}



// A constructor for a temprorary storage of all mails.
function Mailbox(dbFilename,      // <-- The filename where all
		                  //     messages are stored.
		 thisMailboxName, // <-- A string that uniquely
		                  //     identifies this mailbox
		 cb) { // <-- Optional error callback for db.
    if (!isValidDBFilename(dbFilename)) {
	throw new Error('Invalid database filename');
    }
    if (!isValidMailboxName(thisMailboxName)) {
	throw new Error('Invalid mailbox name');
    }

    // How often we should respond with an ack packet.
    this.ackFrequency = 30;
    
    this.dbFilename = dbFilename;
    this.mailboxName = thisMailboxName;
    this.db = new sqlite3.Database(
	dbFilename,
	function(err) {
	    if (err != undefined) {
		cb(err);
	    }
	});

    // For variable visibility.
    var db = this.db;

    // Wait for the creation of all tables to complete before we call cb.
    async.parallel([
	function(a) {initializeSeqNumbersTable(db, a);},
	function(a) {initializePacketsTable(db, a);},
	function(a) {initializeCTable(db, a);},
	function(a) {initializeDiaryNumberTable(db, a);}
    ], function(err) {
	cb(err);
    });
}

/*
  A callback function cb(err, packet) can
  be assigned. It will be called whenever this
  mailbox receives a packet.
  
  By default, there is no callback.
*/
Mailbox.prototype.onPacketReceived = null;

/*
  A callback function cb(err, {"dst":..., "seqnums": ...}) can
  be assigned. It will be called whenever an ack packet
  is received for some packets that this mailbox sent.

  By default, there is no callback.
 */
Mailbox.prototype.onAcknowledged = null;


// Returns the current sequence number stored in the database,
// by calling a callback with that number.
// If no such number exists, it calls the callback without any arguments.
Mailbox.prototype.getCurrentSeqNumber = function(dst, callbackNewNumber) {
    if (!isNonEmptyString(dst)) {
	throw new Error('Dst should be a string. Currently, its value is '+ dst);
    }
    var self = this;
    this.db.serialize(function() {
	self.db.get('SELECT counter FROM seqnumbers WHERE dst = ?', dst,
	   function(err, row) {
	       if (err == undefined) {
		   if (row == undefined) {
		       callbackNewNumber(err);
		   } else {
		       callbackNewNumber(err, row.counter);
		   }
	       } else {
		   callbackNewNumber(err);
	       }
	   });
    });
};

// Makes a new sequence number that can be used.
// Call this method every time we send a packet
Mailbox.prototype.makeNewSeqNumber = function(dst, callbackNewNumber) {
    var self = this;
    var cbNumberRetrived = function(err, x) {
	var makeCompletedFun = function(y) {
	    return function(err) {	
		if (err == undefined) {
		    callbackNewNumber(err, y);
		} else {
		    callbackNewNumber(err);
		}
	    };
	}
	if (x == undefined) {
	    var toReturn = seqnums.make();
	    var nextNumber = seqnums.next(toReturn);
	    self.db.run('INSERT INTO seqnumbers VALUES (?, ?);',
			dst, nextNumber, makeCompletedFun(toReturn));
	} else {
	    var toReturn = x;
	    var nextNumber = seqnums.next(x);
	    self.db.run('UPDATE seqnumbers SET counter = ? WHERE dst = ?',
			nextNumber, dst, makeCompletedFun(toReturn));
	}
    };
    this.getCurrentSeqNumber(dst, cbNumberRetrived);
}

// Gets the last diary number of all messages in THIS box.
Mailbox.prototype.getLastDiaryNumber = function(cb) {
    var query = 'SELECT max(diarynumber) FROM packets';
    this.db.get(query, function(err, result) {
	if (err == undefined) {
	    cb(err, result["max(diarynumber)"]);
	} else {
	    cb(err);
	}
    });
};

// This returns the diary number for a foreign mailbox.
// This number is upon synchronization when we fetch messages from the
// other mailbox.
Mailbox.prototype.getForeignDiaryNumber = function(otherMailbox, cb) {
    if (typeof cb != 'function') {
	throw new Error('cb is of wrong type: ' + cb);
    }
    
    var query = 'SELECT number FROM diarynumbers WHERE mailbox = ?';
    this.db.get(
	query, otherMailbox,
	function(err, row) {
	    if (err == undefined) {
		if (row == undefined) {
		    // Initialize with a low number.
		    cb(err, undefined);
		} else {
		    cb(err, row.number);
		}
	    } else {
		cb(err);
	    }
	});
}

// Use this function to get a number of the first packet to ask for when synchronizing
Mailbox.prototype.getForeignStartNumber = function(otherMailbox, cb) {
    this.getForeignDiaryNumber(otherMailbox, function(err, value) {
	if (err == undefined) {
	    cb(err, (value == undefined? 0 : value));
	} else {
	    cb(err);
	}
    });
}

// Sets the foreign number to a new value.
Mailbox.prototype.setForeignDiaryNumber = function(otherMailbox, newValue, cb) {
    var self = this;
    this.getForeignDiaryNumber(otherMailbox, function(err, previousValue) {
	if (err == undefined) {
	    if (previousValue > newValue) {
		console.log('You are setting a new diary number which is lower than the previous one. This could be a bug.');
	    }

	    if (previousValue == undefined) { // <-- This only happens when there isn't any existing diary number already.
		var query = 'INSERT INTO diarynumbers VALUES (?, ?)';
		self.db.run(query, otherMailbox, newValue, cb);
	    } else {
		var query = 'UPDATE diarynumbers SET number = ? WHERE mailbox = ?';
		self.db.run(query, newValue, otherMailbox, cb);
	    }
	} else {
	    cb(err);
	}
    });
}

// Retrieves the first packet starting from a diary number.
Mailbox.prototype.getFirstPacketStartingFrom = function(diaryNumber, cb) {
    var query = 'SELECT * FROM packets  WHERE ? <= diarynumber ORDER BY diarynumber ASC';
    this.db.get(query, diaryNumber, cb);
}



// Call this method whenever we send or handle a packet.
// (See 'sendPacket' or 'handleIncomingPacket')
// If nothing goes wrong, it calls cb with the new number.
// It doesn't mutate the database.
Mailbox.prototype.makeNewDiaryNumber = function(cb) {
    this.getLastDiaryNumber(function(err, number) {
	if (err == undefined) {
	    var result = (number == undefined?
			  seqnums.make() :
			  seqnums.next(number));
	    cb(err, result);
	} else {
	    cb(err);
	}
    });
};



// Retrieves the C-number for a given (src, dst) pair. A sequence number
// is provided for initialization if no C-number exists. The result is passed to cb.
Mailbox.prototype.getCNumber = function(src, dst, cb) {
    var query = 'SELECT counter FROM ctable WHERE src = ? AND dst = ?';
    var self = this;
    this.db.get(
	query, src, dst,
	function(err, row) {
	    if (err == undefined) {
		if (row == undefined) {
		    cb(err);
		} else {
		    cb(err, row.counter);
		}
	    } else {
		cb(err);
	    }
	});
};


Mailbox.prototype.insertCTable = function(src, dst, value, cb) {
    var insert = 'INSERT INTO ctable VALUES (?, ?, ?)';
    this.db.run(insert, src, dst, value, cb);
};

// Used when sending new packets.
Mailbox.prototype.getOrMakeCNumber = function(dst, seqNumber, cb) {
    var self = this;
    this.getCNumber(
	this.mailboxName, dst,
    	function(err, value) {
	    if (err == undefined) {
		if (value == undefined) { /* If there isn't already a cnumber,
					     initialize it with sequence counter value */
		    self.insertCTable(
			this.mailboxName, dst, seqNumber,
			function(err) {
			    cb(err, seqNumber);
			});
		} else { /* If there is a value, just use it as cnumber.*/
		    cb(err, value);
		}
	    } else {
		cb(err);
	    }
	});
}

Mailbox.prototype.removeObsoletePackets = function(src, dst, cb) {
    this.getCNumber(
	src, dst,
	function(err, value) {
	    if (err == undefined) {
	    } else {
	    }
	});
}

// Update the C table. Used when handling incoming packets.
Mailbox.prototype.updateCTable = function(src, dst, newValue, cb) {
    var onUpdate = function(err) {
	//this.removeObsoletePackets(src, dst, cb);
	cb(err);
    };
    
    var onUpdate = cb;
    var self = this;
    this.getCNumber(src, dst, function(err, currentValue) {
	if (err == undefined) {
	    if (currentValue == undefined) {
		self.insertCTable(src, dst, newValue, onUpdate);
	    } else if (currentValue < newValue) {
		var query = 'UPDATE ctable SET counter = ? WHERE src = ? AND dst = ?';
		self.db.run(query, newValue, src, dst, onUpdate);
	    } else {
		cb(err);
	    }
	} else {
	    cb(err);
	}
    });
};

// Check if an incoming packet should be admitted.
Mailbox.prototype.isAdmissable = function(src, dst, seqNumber, cb) {
    this.getCNumber(src, dst, function(err, cnumber) {
	if (err == undefined) {
	    cb(err, (cnumber == undefined? true : (cnumber <= seqNumber)));
	} else {
	    cb(err);
	}
    });
};


// A packet can be uniquely identified by its source mailbox and the seqNumber.
Mailbox.prototype.hasPacket = function(src, seqNumber, cb) {
    var query = 'SELECT * FROM packets WHERE src = ? AND seqnumber = ?';
    this.db.get(query, src, seqNumber, function(err, row) {
	if (err == undefined) {
	    cb(err, !(row == undefined));
	} else {
	    cb(err);
	}
    });
}


// This method will update the C-table and save the packet in the db.
Mailbox.prototype.registerPacketData = function(packet, cb) {
    var self = this;
    this.hasPacket(packet.src, packet.seqNumber, function(err, has) {
	if (err == undefined) {
	    if (has) {
		
		// Nothing to do if we already have the packet.
		// TODO: If we end up here, we have probably transferred
		//       packet data for no use.
		cb(err);
		
	    } else {

		// Get a diary number for this packet
		self.makeNewDiaryNumber(function(err, num) {
		    if (err == undefined) {

			// Insert the packet into the packet database
			var query = 'INSERT INTO packets VALUES (?, ?, ?, ?, ?, ?, ?, ?)';
			self.db.run(
			    query, num,
			    packet.src, packet.dst, packet.seqNumber,
			    packet.cNumber, packet.label, packet.data, false,
			    function(err) {
				if (err == undefined) {
				    
				    // Update the c-number
				    self.updateCTable(
					packet.src, packet.dst,
					packet.cNumber, cb);
				    
				} else {
				    cb(err);
				}
			    });
		    } else {
			cb(err);
		    }
		});
	    }
	} else {
	    cb(err);
	}
    });
}

// Get the number of packets for which we haven't sent an ack packet.
Mailbox.prototype.getNonAckCount = function(src, cb) {
    var query = 'SELECT count(*) FROM packets WHERE src = ? AND dst = ? AND ack = 0';
    this.db.get(
	query, src, this.mailboxName,
	function(err, row) {
	    if (err == undefined) {
		var value = row['count(*)'];
		if (value == undefined) {
		    cb(err, 0);
		} else {
		    cb(err, value);
		}
	    } else {
		cb(err);
	    }
	});
}

// Set packets as acknowledged
Mailbox.prototype.setAcked = function(src, dst, seqnums, cb) {
    var query = 'UPDATE packets SET ack = 1 WHERE src = ? AND dst = ? AND seqnubmer = ?';
    var self = this;
    var f = function(seqnum, a) {
	self.db.run(query, src, dst, seqnum, a);
    };
    async.map(seqnums, f, cb);
}

function valueOf(x) {
    console.log('     value is %j', x);
    return x;
}


// Sends an ack to the source of a packet.
Mailbox.prototype.sendAck = function(src, cb) {
    console.log('SEND ACK!!');
    var self = this;
    var query = 'SELECT seqnumber FROM packets WHERE src = ? AND dst = ? AND ack = 0';
    self.db.all(
	query, src, self.mailboxName,
	function(err, data) {
	    var seqnums = new Array(data.length);
	    for (var i = 0; i < data.length; i++) {
		seqnums[i] = data[i].seqnumber;
	    }

	    console.log('seqnums = ' + seqnums);
	    
	    async.parallel([
		function(a) {
		    self.sendPacket(
			src/*back to the source*/,
			'ack',
			seqnums,
			a);
		},
		function(a) {
		    self.setAcked(src, self.mailboxName, seqnums, a);
		}], function(err, results) {
		    console.log('Both sending the packet and setAcked completed, with results ' + results);
		    cb(err);
		});
	});
}

// Sends an ack-packet if we have received enough packets.
Mailbox.prototype.sendAckIfNeeded = function(src, cb) {
    var self = this;
    this.getNonAckCount(src, function(err, count) {
	if (err == undefined) {
	    if (count < self.ackFrequency) {
		cb(err);
	    } else {
		self.sendAck(src, cb);
	    }
	} else {
	    cb(err);
	}
    });
}

Mailbox.prototype.maximizeCNumber = function(dst, cb) {
    var update = function(x) {
	self.updateCTable(
	    self.mailboxName,
	    dst,
	    x,
	    cb);
    };
    
    // retrieve the first seqnumber that has not been acked.
    var query = 'SELECT cnumber FROM packets WHERE ack = 0 ORDER BY seqnumber ASC';
    var self = this;
    this.db.get(query, function(err, row) {
	if (err == undefined) {
	    if (row == undefined) { // No packets found, set it to 1 + the latest ack
		var query = 'SELECT cnumber FROM packets WHERE ack = 1 ORDER BY seqnumber DESC';
		self.db.get(query, function(err, row) {
		    if (err == undefined) {
			if (row == undefined) {
			    cb();
			} else {
			    update(1 + row.seqnumber);
			}
		    } else {
			cb(err);
		    }
		});
	    } else {
		update(row.seqnumber);
	    }
	} else {
	    cb(err);
	}
    });
}

Mailbox.prototype.handleAckPacketIfNeeded = function(packet, cb) {
    if (packet.label == 'ack') {
	var self = this;
	self.setAcked(
	    self.mailboxName, packet.src, packet.data,
	    function (err) {
		if (err == undefined) {
		    self.maximizeCNumber(packet.src, cb);
		} else {
		    cb(err);
		}
	    });
    } else {
	cb();
    }
}

// This method is called only for packets that should not be rejected.
Mailbox.prototype.acceptIncomingPacket = function(packet, cb) {
    var self = this;
    this.registerPacketData(packet, function(err) {
	if (err == undefined) {
	    self.handleAckPacketIfNeeded(
		packet, function(err) {
		    self.sendAckIfNeeded(packet.src, cb);
		});
	} else {
	    cb(err);
	}
    });
}

// Handle an incoming packet.
Mailbox.prototype.handleIncomingPacket = function(packet, cb) {
    var self = this;
    this.isAdmissable(
	packet.src,
	packet.dst,
	packet.seqNumber,
	function(err, p) {
	    if (err == undefined) {
		if (p) {
		    self.acceptIncomingPacket(packet, cb);
		} else {
		    cb(err);
		}
	    } else {
		cb(err);
	    }
	});
}



Mailbox.prototype.getDiaryAndSeqNumbers = function(dst, cb) {
    var self = this;
    async.parallel({
	diaryNumber: function(a) {
	    self.makeNewDiaryNumber(a);
	},
	sequenceNumber: function(a) {
	    self.makeNewSeqNumber(dst, a);
	}}, cb);
}


// Given destination mailbox, label and data,
// a new packet is produced that is put in the packets table.
Mailbox.prototype.sendPacket = function (dst, label, data, cb) {
    console.log('Send a packet');
    var self = this;
    this.getDiaryAndSeqNumbers(
	dst,
	function(err, results) {
	    console.log('  generated numbers dnum = ', results.diaryNumber, '  seqnum = ', results.sequenceNumber);
	    if (err == undefined) {
		box.getOrMakeCNumber(
		    dst, results.sequenceNumber,
		    function(err, cNumber) {
			// Now we have all we need to make the packet.
			console.log('Run query');
			var query = 'INSERT INTO packets VALUES (?, ?, ?, ?, ?, ?, ?, ?)';
			self.db.run(
			    query, results.diaryNumber,
			    self.mailboxName, dst, results.sequenceNumber,
			    cNumber, label, data, false,/*not yet acknowledged*/
			    cb);
		    });
	    } else {
		console.log('Error in sendPacket');
		cb(err);
	    }
	});
    console.log('Leaving function sendPacket');
};



function errThrow(err) {
    if (err != undefined) {
	throw new Error('Something wen wrong');
    }
}









//// DEMO

function sendPacketDemo(box) {
    box.sendPacket('dst', 'some-label', 'some-data',
		   function(err) {
		       if (err == undefined) {
			   console.log('Successfully sent the FIRST packet');
			   dispAllTableData(box.db);
			   console.log('Let us send another packet');
			   box.sendPacket('dst', 'some-other-label', 'some-other-data', function(err) {
			       if (err == undefined) {
				   console.log('Successfully sent the SECOND packet');
				   dispAllTableData(box.db);
			       }else {
				   console.log('Failed to send the other packet but at least the first packet succeeded.');
			       }
			   });
		       } else {
			   console.log('Failed to send packet');
		       }
		   });
}

function getCNumberDemo(box) {
    box.getOrMakeCNumber('abra', '12349', function(err, cnumber) {
	console.log('C-number is ' + cnumber);
	box.getOrMakeCNumber('abra', '19999', function(err, cnumber) {
	    console.log('C-number is ' + cnumber);
	});
    });
}

function getLastDiaryNumberDemo(box) {
    box.getLastDiaryNumber(function(err, num) {
	errThrow(err);
	console.log('Last diary number is %j', num);
	box.makeNewDiaryNumber(function(err, num) {
	    console.log('A new number is ' + num);
	    dispAllTableData(box.db);
	});
    });
}

function insertTestPacketDemo(box) {
    box.db.run('INSERT INTO packets VALUES (?, ?, ?, ?, ?, ?, ?, ?)',
    	       129, "abra", "kadabra", 119, 109, "testpacket", "sometestdata", false,
    	       function(err) {
    		   errThrow(err);
    		   box.getLastDiaryNumber(function(err, num) {
    		       errThrow(err);
    		       console.log('Last diary number is %j', num);
		       dispAllTableData(box.db);
    		   });
    	       });
}

function seqNumberDemo(box) {
    box.makeNewSeqNumber('abra', function(err, x) {
	console.log('First seq num is ' + x);
	box.makeNewSeqNumber('abra', function(err, x) {
	    console.log('Second seq num is ' + x);
	    box.makeNewSeqNumber('abra', function(err, y) {
		console.log('Third seq num is ' + y);
	    });
	});
    });
}

function foreignDiaryNumberDemo(box) {
    box.getForeignDiaryNumber('rulle', function(err, value) {
	assert(value == undefined);
	console.log('The diary number is ', value);
	box.setForeignDiaryNumber('rulle', 119, function(err) {
	    box.getForeignDiaryNumber('rulle', function(err, value2) {
		assert(value2 == 119);
		console.log('Now the diary number is ' + value2);
		box.setForeignDiaryNumber('rulle', 135, function(err) {
		    box.getForeignDiaryNumber('rulle', function(err, value3) {
			assert(value3 == 135);
			console.log('The diary number should now be something different: ' + value3);
		    });
		});
	    });
	});
    });
}

function packetsStartingFromDemo(box) {
    box.getFirstPacketStartingFrom(0, function(err, result) {
	console.log('Got this packet: %j', result);
	assert(result == undefined);
	box.sendPacket('dst', 'label', 'data', function(err) {
	    box.getFirstPacketStartingFrom(0, function(err, result) {
		assert(result != undefined);
		console.log('Got THIS packet starting from 0, should be NONEMPTY: %j', result);
		box.getFirstPacketStartingFrom(result.diarynumber + 1, function(err, result) {
		    assert(result == undefined);
		    console.log('This result should be empty: ', result);
		});
	    });
	});
    });
}

function updateCTableDemo(box) {
    box.updateCTable('a', 'b', 19, function(err) {
	box.getCNumber('a', 'b', function(err, value) {
	    assert(value == 19);
	    console.log('The cnumber is ' + value);
	    box.updateCTable('a', 'b', 29, function(err) {
		box.getCNumber('a', 'b', function(err, value) {
		    assert(value == 29);
		    console.log('The cnumber is ' + value);
		    box.updateCTable('a', 'b', 13, function(err) {
			box.getCNumber('a', 'b', function(err, value) {
			    assert(value == 29);
			    console.log('Should be unchanged: ', value);
			});
		    });
		});
	    });
	});
    });
}

function registerPacketDataDemo(box) {
    box.registerPacketData(
	new pkt.Packet('a', 'b', 119, 30, 'My label', 'Some data'), function(err) {
	    assert(err == undefined);
	    box.getCNumber('a', 'b', function(err, num) {
		assert(err == undefined);
		assert(num == 30);
		box.getLastDiaryNumber(function(err, dnum) {
		    assert(err == undefined);
		    box.getFirstPacketStartingFrom(dnum, function(err, packet) {
			assert(packet.src == 'a');
			assert(packet.dst == 'b');
			assert(packet.seqnumber == 119);
			assert(packet.cnumber == 30);
			assert(packet.label == 'My label');
			assert(packet.data == 'Some data');
			console.log('SUCCESS');
		    });
		});
	    });
	});
}


function maximizeCNumberDemo(box) {
    var spammer = function(n, cb) {
	if (n == 0) {
	    console.log('Done spamming');
	    cb();
	} else {
	    console.log('Handle packet ' + n);
	    box.handleIncomingPacket(
		new pkt.Packet(
		    'some-spammer', box.mailboxName,
		    n, -1, 'Spam message', 'There are ' + n + ' messages left to send'),

		// Function to be called once the incoming packet has been handled:
		function(err) {
		    spammer(n - 1, cb);
		});
	}
    };
    spammer(30, function(err) {
	assert(err == undefined);
	dispAllTableData(box.db);
    });
}

var inMemory = true;
var filename = (inMemory? ':memory:' : 'demo.db');
var box = new Mailbox(filename, 'demobox', function(err) {
    maximizeCNumberDemo(box);
    //registerPacketDataDemo(box);
    //updateCTableDemo(box);
    //packetsStartingFromDemo(box);
    //foreignDiaryNumberDemo(box);
    //sendPacketDemo(box);
    //getLastDiaryNumberDemo(box);
});


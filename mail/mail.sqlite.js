/*

Mailbox model based on sqlite

*/

var TransactionDatabase = require("sqlite3-transactions").TransactionDatabase;
var sqlite3 = require('sqlite3').verbose();
var seqnums = require('./seqnums.js');
var async = require('async');
var pkt = require('./packet.js');
var intarray = require('./intarray.js');
var assert = require('assert');



function expand(span, value) {
    if (span == undefined) {
	return [value, value];
    } else {
	return [Math.min(span[0], value),
	        Math.max(span[1], value)];
    }
}

/////////////////////////////////////////////////////////
// General functions for checking if an object is a string
function isString(x) {
    return typeof x == 'string';
}

function isNonEmptyString(x) {
    return isString(x) && 0 < x.length;
}

function isFunction(x) {
    return typeof x == 'function';
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

// To obtain these strings, instantiate the db with the file 'network.db'
// Then type in the terminal 'sqlite3 network.db .fullschema'
//
// PRIMARY KEY should be the last column of every create statement
var fullschema = "CREATE TABLE IF NOT EXISTS seqnumbers (dst TEXT, counter BIGINT, PRIMARY KEY(dst));\
                  CREATE TABLE IF NOT EXISTS packets (diarynumber BIGINT, src TEXT, dst TEXT, \
                          seqnumber BIGINT, cnumber BIGINT, label TEXT, data BLOB, ack INTEGER, PRIMARY KEY(diarynumber)); \
                  CREATE TABLE IF NOT EXISTS diarynumbers (mailbox TEXT, number BIGINT, PRIMARY KEY(mailbox)); \
                  CREATE TABLE IF NOT EXISTS ctable (src TEXT, dst TEXT, counter BIGINT, PRIMARY KEY(src, dst));";


function createAllTables(db, cb) {
    assert(isFunction(cb));
    db.exec(fullschema, cb);
}

function getAllTables(db, cb) {
    assert(isFunction(cb));    
    db.all('SELECT * FROM sqlite_master WHERE type=\'table\'', cb);
}

function getAllTableRows(db, tableName, cb) {
    assert(isFunction(cb));    
    db.all('SELECT * FROM ' + tableName, cb);
}


// For debugging. The callback cb is optional
function dispAllTableData(db, cb) {
    assert(isFunction(cb));    
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




// A constructor for a temporary storage of all mails and their
// transactions.
function Mailbox(dbFilename,      // <-- The filename where all
		                  //     messages are stored.
		 thisMailboxName, // <-- A string that uniquely
		                  //     identifies this mailbox
		 cb) {
    assert(isFunction(cb));    
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
    this.db = new TransactionDatabase(
	new sqlite3.Database(
	    dbFilename,
	    function(err) {
		if (err != undefined) {
		    cb(err);
		}
	    }
	)
    );

    // For variable visibility.
    var db = this.db;

    createAllTables(db, cb);
}

/*
  A callback function cb(packet) can
  be assigned. It will be called whenever this
  mailbox receives a packet.
  
  By default, there is no callback.
*/
Mailbox.prototype.onPacketReceived = null;

/*
  A callback function cb({"dst":..., "seqnums": ...}) can
  be assigned. It will be called whenever an ack packet
  is received for some packets that this mailbox sent.

  By default, there is no callback.
 */
Mailbox.prototype.onAcknowledged = null;


// Returns the current sequence number stored in the database,
// by calling a callback with that number.
// If no such number exists, it calls the callback without any arguments.
Mailbox.prototype.getCurrentSeqNumber = function(dst, callbackNewNumber) {
    assert(isFunction(callbackNewNumber));    
    if (!isNonEmptyString(dst)) {
	throw new Error('Dst should be a string. Currently, its value is ' + dst);
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


function makeNewSeqNumberSub(T, dst, x, cb) {
    assert(isFunction(cb));    
    var self = this;
    var makeCompletedFun = function(y) {
	return function(err) {
	    T.commit(
		function(err) {
		    if (err == undefined) {
			cb(err, y);
		    } else {
			cb(err);
		    }
		}
	    );
	};
    };
    if (x == undefined) {
	var toReturn = seqnums.make();
	var nextNumber = seqnums.next(toReturn);
	T.run('INSERT INTO seqnumbers VALUES (?, ?);',
	      dst, nextNumber, makeCompletedFun(toReturn));
    } else {
	var toReturn = x;
	var nextNumber = seqnums.next(x);
	T.run('UPDATE seqnumbers SET counter = ? WHERE dst = ?',
	      nextNumber, dst, makeCompletedFun(toReturn));
    }

}

// Makes a new sequence number that can be used.
// Call this method every time we send a packet
Mailbox.prototype.makeNewSeqNumber = function(dst, cb) {
    assert(isFunction(cb));    
    this.db.beginTransaction(
	function(err, T) {
	    assert(err == undefined);
	    assert(T != undefined);
	    T.get(
		'SELECT counter FROM seqnumbers WHERE dst = ?', dst,
		function(err, row) {
		    if (err == undefined) {
			if (row == undefined) {
			    makeNewSeqNumberSub(T, dst, undefined, cb);
			} else {
			    makeNewSeqNumberSub(T, dst, row.counter, cb);
			}
		    } else {
			cb(err);
		    }
		}
	    );
	}
    );    
}

// Gets the last diary number of all messages in THIS box.
Mailbox.prototype.getLastDiaryNumber = function(cb) {
    assert(isFunction(cb));    
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
    assert(isFunction(cb));    
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
    assert(isFunction(cb));    
    this.getForeignDiaryNumber(otherMailbox, function(err, value) {
	if (err == undefined) {
	    cb(err, (value == undefined? 0 : value));
	} else {
	    cb(err);
	}
    });
}

// Sets the foreign number to a new value.

// TODO: getForeignDiaryNumber and the following query should be in one transaction,
// just like inside makeNewSeqNumber.
Mailbox.prototype.setForeignDiaryNumber = function(otherMailbox, newValue, cb) {
    assert(isFunction(cb));    
    var self = this;
    this.getForeignDiaryNumber(otherMailbox, function(err, previousValue) {
	if (err == undefined) {
	    if (previousValue > newValue) {
		console.log('You are setting a new diary number which is lower than the previous one. This could be a bug.');
	    }

	    if (previousValue == undefined) {  // <-- This only happens when
		                               //     there isn't any existing
		                               //     diary number already.
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
Mailbox.prototype.getFirstPacketStartingFrom = function(diaryNumber, lightWeight, cb) {
    assert(isFunction(cb));
    // During the synchronization process, we might only want the essential information
    // to determine whether or not we are going to ask for the whole packet.
    var what = (lightWeight? 'src,seqnumber' : '*');
    
    var query = 'SELECT ' + what +
	' FROM packets  WHERE ? <= diarynumber ORDER BY diarynumber ASC';
    
    this.db.get(query, diaryNumber, cb);
}



// Call this method whenever we send or handle a packet.
// (See 'sendPacket' or 'handleIncomingPacket')
// If nothing goes wrong, it calls cb with the new number.
// It doesn't mutate the database.
Mailbox.prototype.makeNewDiaryNumber = function(cb) {
    assert(isFunction(cb));    
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
    assert(isFunction(cb));    
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
    assert(isFunction(cb));    
    var insert = 'INSERT INTO ctable VALUES (?, ?, ?)';
    this.db.run(insert, src, dst, value, cb);
};

// Used when sending new packets.
Mailbox.prototype.getOrMakeCNumber = function(dst, seqNumber, cb) {
    assert(isFunction(cb));    
    var self = this;
    this.getCNumber(
	self.mailboxName, dst,
    	function(err, value) {
	    if (err == undefined) {
		if (value == undefined) { /* If there isn't already a cnumber,
					     initialize it with sequence counter value */
		    self.insertCTable(
			self.mailboxName, dst, seqNumber,
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
    assert(isFunction(cb));    
    var self = this;
    this.getCNumber(
	src, dst,
	function(err, value) {
	    if (err == undefined) {
		var query = 'DELETE FROM packets WHERE seqnumber < ? AND src = ? AND dst = ?';
		self.db.run(query, value, src, dst, cb);
	    } else {
		cb(err);
	    }
	});
}

Mailbox.prototype.getTotalPacketCount = function(cb) {
    assert(isFunction(cb));    
    var query = 'SELECT count(*) FROM packets';
    this.db.get(
	query, function(err, row) {
	    if (err == undefined) {
		cb(err, row['count(*)']);
	    } else {
		cb(err);
	    }
	}
    );
}



// Update the C table. Used when handling incoming packets.
Mailbox.prototype.updateCTable = function(src, dst, newValue, cb) {
    assert(isFunction(cb));
    var self = this;
    var onUpdate = function(err) {
	self.removeObsoletePackets(src, dst, cb);
    };
    
    var self = this;
    this.getCNumber(src, dst, function(err, currentValue) {
	if (err == undefined) {
	    if (currentValue == undefined) {
		self.insertCTable(src, dst, newValue, onUpdate);
	    } else if (currentValue < newValue) {
		var query = 'UPDATE ctable SET counter = ? WHERE src = ? AND dst = ?';
		console.log('Update ctable from ' + currentValue + ' to '
			    + newValue + ' at (' + src + ', ' + dst + ')');
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
Mailbox.prototype.isAdmissible = function(src, dst, seqNumber, cb) {
    assert(isFunction(cb));    
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
    assert(isFunction(cb));    
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
    assert(isFunction(cb));    
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
    assert(isFunction(cb));    
    var query = 'UPDATE packets SET ack = 1 WHERE src = ? AND dst = ? AND seqnumber = ?';
    var self = this;

    var setter = function(nums) {
	if (nums.length == 0) {
	    cb();
	} else {
	    var seqnum = nums[0];
	    self.db.run(
		query, src, dst, seqnum,
		function(err) {
		    if (err == undefined) {
			setter(nums.slice(1));
		    } else {
			cb(err);
		    }
		});
	}
    };
    setter(seqnums);
}

function valueOf(x) {
    console.log('     value is %j', x);
    return x;
}


// Sends an ack to the source of a packet.
Mailbox.prototype.sendAck = function(src, cb) {
    assert(isFunction(cb));    
    var self = this;
    var query = 'SELECT seqnumber FROM packets WHERE src = ? AND dst = ? AND ack = 0';
    self.db.all(
	query, src, self.mailboxName,
	function(err, data) {
	    var seqnums = new Array(data.length);
	    for (var i = 0; i < data.length; i++) {
		seqnums[i] = data[i].seqnumber;
	    }
	    console.log('Send ack for ' + seqnums.length + ' packets.');
	    self.sendPacket(
		src/*back to the source*/,
		'ack',
		intarray.serialize(seqnums),
		function(err) {
		    if (err == undefined) {
			self.setAcked(
			    src, self.mailboxName, seqnums,
			    function(err) {
				cb(err);
			    });
		    } else {
			cb(err);
		    }
		});
	});
}

// Sends an ack-packet if we have received enough packets.
Mailbox.prototype.sendAckIfNeeded = function(src, cb) {
    assert(isFunction(cb));    
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
    assert(isFunction(cb));    
    var update = function(x) {

		self.updateCTable(
		    self.mailboxName,
		    dst,
		    x,
		    cb);
	
	// self.getCNumber(
	//     self.mailboxName, dst,
	//     function (err, oldValue) {
	// 	console.log('%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% Move C-number from ', oldValue, ' to ', x, ' which is ', x - oldValue);
	// 	self.updateCTable(
	// 	    self.mailboxName,
	// 	    dst,
	// 	    x,
	// 	    cb);
	//     }
	// );		
	
    };
    
    // retrieve the first seqnumber that has not been acked.
    var query = 'SELECT seqnumber FROM packets WHERE ack = 0 ORDER BY seqnumber ASC';
    var self = this;
    this.db.get(query, function(err, row) {
	if (err == undefined) {
	    if (row == undefined) { // No packets found, set it to 1 + the latest ack
		var query = 'SELECT seqnumber FROM packets WHERE ack = 1 ORDER BY seqnumber DESC';
		self.db.get(query, function(err, row) {
		    if (err == undefined) {
			if (row == undefined) {

			    // No packets, so let the C number remain the same, whatever it was.
			    cb();
			    
			} else {

			    // The last packet that was acked + 1, in case no packets with ack=0
			    update(1 + row.seqnumber);
			}
		    } else {
			cb(err);
		    }
		});
	    } else {

		// The first packet not acked.
		update(row.seqnumber);
	    }
	} else {
	    cb(err);
	}
    });
}

Mailbox.prototype.handleAckPacketIfNeeded = function(packet, cb) {
    assert(isFunction(cb));
    var self = this;
    if (packet.label == 'ack' && packet.dst == this.mailboxName) {

	//self.dispPacketSummary(
	    //function(err) {
		var seqnums = intarray.deserialize(packet.data);

		console.log('Incoming ack packet with ' + seqnums.length + ' seqnums');
		

		// Optional call to function whenever some packets that we sent were acknowledged.
		if (this.onAcknowledged != undefined) {
		    this.onAcknowledged({
			dst: packet.src, // The mailbox we sent to
			seqnums: seqnums // The sequence numbers.
		    });
		}
		
		self.setAcked(
		    self.mailboxName, packet.src,
		    seqnums,
		    function (err) {
			if (err == undefined) {
			    self.maximizeCNumber(packet.src, cb);
			} else {
			    cb(err);
			}
		    });
//	    }
	//);

    } else {
	cb();
    }
}

// This method is called only for packets that should not be rejected.
Mailbox.prototype.acceptIncomingPacket = function(packet, cb) {
    assert(isFunction(cb));    
    var self = this;

    // This call will
    //  * Store the packet in the mailbox
    //  * Update the C-table using the data of the packet
    this.registerPacketData(packet, function(err) {
	if (err == undefined) {

	    // Optional callback to inform us that
	    // we have a new packet to open.
	    if (self.onPacketReceived != undefined
		&& packet.dst == self.mailboxName) {
		self.onPackedReceived(packet);
	    }
	    
	    // If the packet was intended for this mailbox,
	    // this call will mark packets as acknowledged
	    // and maximize the C-number.
	    self.handleAckPacketIfNeeded(
		packet, function(err) {

		    // If we have received enough packets
		    // from packet.src, return an 'ack' packet
		    // to that source.
		    self.sendAckIfNeeded(packet.src, cb);
		    
		});
	} else {
	    cb(err);
	}
    });
}

// Handle an incoming packet.
Mailbox.prototype.handleIncomingPacket = function(packet, cb) {
    assert(isFunction(cb));    
    var self = this;
    this.isAdmissible(
	packet.src,
	packet.dst,
	packet.seqNumber,
	function(err, p) {
	    assert(err == undefined);
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
    assert(isFunction(cb));    
    var self = this;
    self.makeNewDiaryNumber(function(err, diaryNumber) {
	if (err == undefined) {
	    self.makeNewSeqNumber(dst, function(err, seqNumber) {
		if (err == undefined) {
		    assert(typeof diaryNumber == 'number');
		    assert(typeof seqNumber == 'number');
		    cb(err, {diaryNumber: diaryNumber, seqNumber: seqNumber});
		} else {
		    cb(err);
		}
	    });
	} else {
	    cb(err);
	}
    });
}

Mailbox.prototype.dispPacketSummary = function(cb) {
    var self = this;
    this.db.all(
	'SELECT diarynumber, seqnumber, ack FROM packets',
	function (err, results) {
	    if (err == undefined) {
		console.log('PACKET SUMMARY OF ' + self.mailboxName + ' (' + results.length + ' packets)');
		for (var i = 0; i < results.length; i++) {
		    var r = results[i];
		    console.log('  diarynumber = ' + r.diarynumber +
				'   seqnumber = ' + r.seqnumber + '   ack = ' + r.ack);
		}
		cb(err);
	    } else {
		cb(err);
	    }
	}
    );
}



// Given destination mailbox, label and data,
// a new packet is produced that is put in the packets table.
Mailbox.prototype.sendPacket = function (dst, label, data, cb) {
    assert(isFunction(cb));    
    var self = this;
    if ((typeof data != 'string') && (typeof data != 'object')) {
	cb(new Error('Please only send data in the form of a Buffer'));
    } else {
	if (typeof data == 'string') {
	    console.log('It is recommended that the data you store is Buffer. Use string only for debugging.');
	}
	this.getDiaryAndSeqNumbers(
	    dst,
	    function(err, results) {
		var seqNumber = results.seqNumber;
		if (err == undefined) {
		    self.getOrMakeCNumber(
			dst, results.seqNumber,
			function(err, cNumber) {
			    // Now we have all we need to make the packet.
			    var query = 'INSERT INTO packets VALUES (?, ?, ?, ?, ?, ?, ?, ?)';
			    self.db.run(
				query, results.diaryNumber,
				self.mailboxName, dst, results.seqNumber,
				cNumber, label, data, false,/*not yet acknowledged*/
				cb);
			});
		} else {
		    cb(err);
		}
	    });
    }
};




module.exports.Mailbox = Mailbox;
module.exports.dispAllTableData = dispAllTableData;
module.exports.expand = expand;

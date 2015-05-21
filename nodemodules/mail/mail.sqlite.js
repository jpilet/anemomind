/*

  Mailbox model based on sqlite

  TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO 
  TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO 
  TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO 
  TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO

       WHAT OTHER OPERATIONS NEED TO BE WRAPPED INSIDE TRANSACTIONS TO PREVENT
       PROBLEMS WITH CONCURRENT ACCESS TO THE DB? This is a very realistic
       scenario, e.g. if we on the anemobox decide to send a file while the mailbox
       is being synchronized over bluetooth.
  
  TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO 
  TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO 
  TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO 
  TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO TODO 
*/

var TransactionDatabase = require("sqlite3-transactions").TransactionDatabase;
var sqlite3 = require('sqlite3').verbose();
var async = require('async');
var assert = require('assert');
var pkt = require('./packet.js');
var bigint = require('./bigint.js');
var labels = require('./labels.js');

var ACKLABEL = labels.ack;

function serializeString(x) {
  assert(typeof x == 'string');
  var buf = new Buffer(2*x.length);
  var len = buf.write(x, 0);
  return buf.slice(0, len);
}


function serializeSeqNums(x) {
  return bigint.serialize(x);
}

function deserializeSeqNums(x) {
  return bigint.deserializeBigInts(x, bigint.defaultWidth);
}

function makeNestedLogger() {
  var indent = 0;
  return function(s) {
    var x = '';
    for (var i = 0; i < indent; i++) {
      x = x + '  ';
    }
    console.log(x + s);
    indent++;
  };
}

function isValidOrUndefined(x, tester) {
  if (x == undefined) {
    return true;
  } else {
    return tester(x);
  }
}




function expand(span, value) {
  if (span == undefined) {
    return [value, value];
  } else {
    var newMin = (value < span[0]? value : span[0]);
    var newMax = (span[1] < value? value : span[1]);
    return [newMin, newMax];
  }
}

// Such as sequence numbers
function isCounter(x) {
  return bigint.isBigInt(x);
}

// Such as mailbox names
function isIdentifier(x) {
  return typeof x == 'string';
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

function isObject(x) {
  return typeof x == 'object';
}

function isNumber(x) {
  return typeof x == 'number';
}

// Also check that the types are valid types.
function isValidPacketSub(x) {
  if (pkt.isFullPacket(x)) {
    return isValidOrUndefined(x.src, isIdentifier) &&
      isValidOrUndefined(x.dst, isIdentifier) &&
      isValidOrUndefined(x.diaryNumber, isCounter) &&
      isValidOrUndefined(x.seqNumber, isCounter) &&
      isValidOrUndefined(x.label, isNumber) &&
      isValidOrUndefined(x.cNumber, isCounter) &&
      isValidOrUndefined(x.data, isObject);
  }
  return false;
}

function isValidPacket(x) {
  if (isValidPacketSub(x)) {
    return true;
  }
  return false;
}



/////////////////////////////////////////////////////////
function isValidDBFilename(x) {
  return isNonEmptyString(x);
}

function isValidMailboxName(x) {
  return isIdentifier(x);
}

function runWithLog(db, cmd) {
  console.log(cmd);
  db.run(cmd);
}

// To obtain these strings, instantiate the db with the file 'network.db'
// Then type in the terminal 'sqlite3 network.db .fullschema'
//
// PRIMARY KEY should be the last column of every create statement
var fullschema = "CREATE TABLE IF NOT EXISTS seqNumbers (dst TEXT, counter TEXT, PRIMARY KEY(dst));\
CREATE TABLE IF NOT EXISTS packets (diaryNumber TEXT, src TEXT, dst TEXT, \
seqNumber TEXT, cNumber TEXT, label INT, data BLOB, ack INTEGER, PRIMARY KEY(diaryNumber)); \
CREATE TABLE IF NOT EXISTS diaryNumbers (mailbox TEXT, number TEXT, PRIMARY KEY(mailbox)); \
CREATE TABLE IF NOT EXISTS ctable (src TEXT, dst TEXT, counter TEXT, PRIMARY KEY(src, dst));";

function createAllTables(db, cb) {
  assert(isFunction(cb));
  db.exec(fullschema, cb);
}

function dropTables(db, cb) {
  var names = ['seqNumbers', 'packets', 'diaryNumbers', 'ctable'];
  var query = '';
  for (var i = 0; i < names.length; i++) {
    query += 'DROP TABLE IF EXISTS ' + names[i] + ';';
  }
  db.exec(query, cb);
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




// Don't call this constructor directly: Please call
// tryMakeMailbox instead.
function Mailbox(dbFilename, mailboxName, ackFrequency, db) {
  var goodInput = 
    isString(dbFilename) &&
    isString(mailboxName) &&
    isNumber(ackFrequency) &&
    isObject(db);
  if (!goodInput) {
    console.log('You are trying to create a mailbox with bad inputs.');
    console.log('Dont call this constructor directly, use makeMailbox instead.');
  }
  this.dbFilename = dbFilename;
  this.mailboxName = mailboxName;
  this.ackFrequency = ackFrequency;
  this.db = db;
}

function tryMakeMailbox(dbFilename,  // <-- The filename where all
		        //     messages are stored.
			mailboxName, // <-- A string that uniquely
		        //     identifies this mailbox
			cb) { // <-- call cb(err, mailbox) when the mailbox is created.
  assert(isFunction(cb));    
  if (!isValidDBFilename(dbFilename)) {
    throw new Error('Invalid database filename');
  }
  if (!isValidMailboxName(mailboxName)) {
    throw new Error('Invalid mailbox name');
  }

  var db = new TransactionDatabase(
    new sqlite3.Database(
      dbFilename,
      function(err) {
	if (err != undefined) {
	  cb(err);
	}
      }
    )
  );
  createAllTables(db, function(err) {
    if (err) {
      cb(err);
    } else {
      cb(
	undefined,
	new Mailbox(dbFilename, mailboxName, 30, db)
      );
    }
  });
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
  if (!isIdentifier(dst)) {
    callbackNewNumber(new Error('dst is not an identifier ' + dst));
  } else {
    
    if (!isNonEmptyString(dst)) {
      throw new Error('Dst should be a string. Currently, its value is ' + dst);
    }
    var self = this;
    this.db.serialize(function() {
      self.db.get(
	'SELECT counter FROM seqNumbers WHERE dst = ?', dst,
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
  }
};


function makeNewSeqNumberSub(T, dst, x, cb) {
  assert(isIdentifier(dst));
  assert(isCounter(x) || x == undefined);
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
    var toReturn = bigint.makeFromTime();
    var nextNumber = bigint.inc(toReturn);
    T.run('INSERT INTO seqNumbers VALUES (?, ?);',
	  dst, nextNumber, makeCompletedFun(toReturn));
  } else {
    var toReturn = x;
    var nextNumber = bigint.inc(x);
    T.run('UPDATE seqNumbers SET counter = ? WHERE dst = ?',
	  nextNumber, dst, makeCompletedFun(toReturn));
  }

}

// Makes a new sequence number that can be used.
// Call this method every time we send a packet
Mailbox.prototype.makeNewSeqNumber = function(T, dst, cb) {
  assert(isFunction(cb));
  assert(isIdentifier(dst));
  
  T.get(
    'SELECT counter FROM seqNumbers WHERE dst = ?', dst,
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

// Gets the last diary number of all messages in THIS box.
Mailbox.prototype.getLastDiaryNumber = function(T, cb) {
  assert(isFunction(cb));    
  var query = 'SELECT diaryNumber FROM packets ORDER BY diaryNumber DESC';
  T.get(query, function(err, result) {
    if (err == undefined) {
      cb(err, (result == undefined? undefined : result.diaryNumber));
    } else {
      cb(err);
    }
  });
};

// This returns the diary number for a foreign mailbox.
// This number is upon synchronization when we fetch messages from the
// other mailbox.
Mailbox.prototype.getForeignDiaryNumberSub = function(T, otherMailbox, cb) {
  assert(isIdentifier(otherMailbox));
  assert(isFunction(cb));    
  if (typeof cb != 'function') {
    throw new Error('cb is of wrong type: ' + cb);
  }
  
  var query = 'SELECT number FROM diaryNumbers WHERE mailbox = ?';
  T.get(
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

Mailbox.prototype.getForeignDiaryNumber = function(otherMailbox, cb) {
  var self = this;
  this.db.beginTransaction(function(err, T) {
    assert(err == undefined);
    self.getForeignDiaryNumberSub(T, otherMailbox, function(err, result) {
      T.commit(function(err2) {
	cb(err || err2, result);
      });
    });
  });
};


// Use this function to get a number of the first packet to ask for when synchronizing
Mailbox.prototype.getForeignStartNumber = function(otherMailbox, cb) {
  assert(isIdentifier(otherMailbox));
  assert(isFunction(cb));
  
  this.getForeignDiaryNumber(otherMailbox, function(err, value) {
    if (err == undefined) {
      cb(err, (value == undefined? bigint.zero() : value));
    } else {
      cb(err);
    }
  });
}

// Sets the foreign number to a new value.
Mailbox.prototype.setForeignDiaryNumber = function(otherMailbox, newValue, cb) {
  assert(isFunction(cb));
  assert(isIdentifier(otherMailbox));
  assert(isCounter(newValue));
  
  var self = this;
  self.db.beginTransaction(function(err, T) {
    var cb2 = function(err) {
      T.commit(function(err2) {
	cb(err || err2);
      });
    }
    self.getForeignDiaryNumberSub(T, otherMailbox, function(err, previousValue) {
      if (err == undefined) {
	if (previousValue > newValue) {
	  console.log('You are setting a new diary number which is lower than the previous one. This could be a bug.');
	}

	if (previousValue == undefined) {  // <-- This only happens when
	  //     there isn't any existing
	  //     diary number already.
	  var query = 'INSERT INTO diaryNumbers VALUES (?, ?)';
	  T.run(query, otherMailbox, newValue, cb2);
	} else {
	  var query = 'UPDATE diaryNumbers SET number = ? WHERE mailbox = ?';
	  T.run(query, newValue, otherMailbox, cb2);
	}
      } else {
	cb2(err);
      }
    });
  });
}

// Retrieves the first packet starting from a diary number.
Mailbox.prototype.getFirstPacketStartingFrom = function(diaryNumber, lightWeight, cb) {
  assert(isFunction(cb));
  if (!isCounter(diaryNumber)) {
    cb('The diary number must be a counter value, but was provided with ' + diaryNumber);
  } else {
    // During the synchronization process, we might only want the essential information
    // to determine whether or not we are going to ask for the whole packet.
    var what = (lightWeight? 'diaryNumber,src,seqNumber,dst' : '*');
    
    var query = 'SELECT ' + what +
      ' FROM packets  WHERE ? <= diaryNumber ORDER BY diaryNumber ASC';
    
    this.db.get(query, diaryNumber, cb);
  }
}



// Call this method whenever we send or handle a packet.
// (See 'sendPacket' or 'handleIncomingPacket')
// If nothing goes wrong, it calls cb with the new number.
// It doesn't mutate the database.
Mailbox.prototype.makeNewDiaryNumber = function(T, cb) {
  assert(isFunction(cb));    
  this.getLastDiaryNumber(T, function(err, number) {
    if (err == undefined) {

      var result = (number == undefined?
		    bigint.makeFromTime() : 
		    bigint.inc(number));
      cb(err, result);
    } else {
      cb(err);
    }
  });
};



// Retrieves the C-number for a given (src, dst) pair. A sequence number
// is provided for initialization if no C-number exists. The result is passed to cb.
Mailbox.prototype.getCNumber = function(T, src, dst, cb) {
  assert(isIdentifier(src));
  assert(isIdentifier(dst));
  assert(isFunction(cb));
  var query = 'SELECT counter FROM ctable WHERE src = ? AND dst = ?';
  var self = this;
  T.get(
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


Mailbox.prototype.insertCTable = function(T, src, dst, value, cb) {
  assert(isIdentifier(src));
  assert(isIdentifier(dst));
  assert(isCounter(value));
  assert(isFunction(cb));    
  var insert = 'INSERT INTO ctable VALUES (?, ?, ?)';
  T.run(insert, src, dst, value, cb);
};

// Used when sending new packets.
Mailbox.prototype.getOrMakeCNumber = function(T, dst, seqNumber, cb) {
  assert(isIdentifier(dst));
  assert(isCounter(seqNumber));
  assert(isFunction(cb));    
  var self = this;
  this.getCNumber(T, 
    self.mailboxName, dst,
    function(err, value) {
      if (err == undefined) {
	if (value == undefined) { /* If there isn't already a cNumber,
				     initialize it with sequence counter value */
	  self.insertCTable(T, 
	    self.mailboxName, dst, seqNumber,
	    function(err) {
	      cb(err, seqNumber);
	    });
	} else { /* If there is a value, just use it as cNumber.*/
	  cb(err, value);
	}
      } else {
	cb(err);
      }
    });
}

Mailbox.prototype.removeObsoletePackets = function(T, src, dst, cb) {
  assert(isIdentifier(src));
  assert(isIdentifier(dst));
  assert(isFunction(cb));    
  var self = this;
  this.getCNumber(T,
    src, dst,
    function(err, value) {
      if (err == undefined) {
	var query = 'DELETE FROM packets WHERE seqNumber < ? AND src = ? AND dst = ?';
	T.run(query, value, src, dst, cb);
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
Mailbox.prototype.updateCTable = function(T, src, dst, newValue, cb) {
  assert(isIdentifier(src));
  assert(isIdentifier(dst));
  assert(isCounter(newValue));
  assert(src != dst);
  assert(isFunction(cb));
  var self = this;
  var onUpdate = function(err) {
    assert(err == undefined);
    if (err == undefined) {
      self.removeObsoletePackets(T, src, dst, cb);	    
    } else {
      cb(err);
    }
  };
  
  var self = this;
  this.getCNumber(T, src, dst, function(err, currentValue) {
    if (err == undefined) {
      if (currentValue == undefined) {
	self.insertCTable(T, src, dst, newValue, onUpdate);
      } else if (currentValue < newValue) {
	var query = 'UPDATE ctable SET counter = ? WHERE src = ? AND dst = ?';
	T.run(query, newValue, src, dst, onUpdate);
      } else {
	cb(err);
      }
    } else {
      cb(err);
    }
  });
};

// Check if an incoming packet should be admitted.
Mailbox.prototype.isAdmissibleSub = function(T, src, dst, seqNumber, cb) {
  assert(isIdentifier(src));
  assert(isIdentifier(dst));
  assert(isCounter(seqNumber));
  assert(src != undefined);
  assert(dst != undefined);
  
  assert(isFunction(cb));

  if (src == this.mailboxName) {
    cb(undefined, false);
  } else {
    this.getCNumber(T, src, dst, function(err, cNumber) {
      if (err == undefined) {
	cb(err, (cNumber == undefined? true : (cNumber <= seqNumber)));
      } else {
	cb(err);
      }
    });
  }
};

Mailbox.prototype.isAdmissible = function(src, dst, seqNumber, cb) {
  var self = this;
  this.db.beginTransaction(function(err, T) {
    var cb2 = function(err, adm) {
      T.commit(function(err2) {
	cb(err || err2, adm);
      });
    };

    self.isAdmissibleSub(T, src, dst, seqNumber, cb2);
  });
}



Mailbox.prototype.getAllPackets = function(cb) {
  this.db.all('SELECT * FROM packets', cb);
}

// A packet can be uniquely identified by its source mailbox and the seqNumber.
Mailbox.prototype.hasPacket = function(T, src, seqNumber, cb) {
  assert(isIdentifier(src));
  assert(isCounter(seqNumber));
  assert(isFunction(cb));    
  var query = 'SELECT * FROM packets WHERE src = ? AND seqNumber = ?';
  T.get(query, src, seqNumber, function(err, row) {
    if (err == undefined) {
      cb(err, !(row == undefined));
    } else {
      cb(err);
    }
  });
}


// This method will update the C-table and save the packet in the db.
Mailbox.prototype.registerPacketData = function(T, packet, cb) {
  assert(isValidPacket(packet));
  var logger = makeNestedLogger();
  assert(isFunction(cb));    
  var self = this;
  this.hasPacket(T, packet.src, packet.seqNumber, function(err, has) {
    if (err == undefined) {
      if (has) {

	// Nothing to do if we already have the packet.
	// TODO: If we end up here, we have probably transferred
	//       packet data for no use.
	cb(err);
	
      } else {

	// Get a diary number for this packet
	self.makeNewDiaryNumber(T, function(err, num) {
	  if (err == undefined) {


	    // Insert the packet into the packet database
	    var query = 'INSERT INTO packets VALUES (?, ?, ?, ?, ?, ?, ?, ?)';
	    T.run(
	      query, num,
	      packet.src, packet.dst, packet.seqNumber,
	      packet.cNumber, packet.label, packet.data, false,
	      function(err) {

		
		if (err == undefined) {

		  
		  // Update the c-number
		  self.updateCTable(T,
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
  assert(isIdentifier(src));
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
Mailbox.prototype.setAcked = function(T, src, dst, seqnums, cb) {
  assert(isIdentifier(src));
  assert(isIdentifier(dst));
  assert(isFunction(cb));    
  var query = 'UPDATE packets SET ack = 1 WHERE src = ? AND dst = ? AND seqNumber = ?';
  var self = this;

  var setter = function(nums) {
    if (nums.length == 0) {
      cb();
    } else {
      var seqnum = nums[0];
      T.run(
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



// Sends an ack to the source of a packet.
Mailbox.prototype.sendAck = function(T, src, cb) {
  assert(isIdentifier(src));
  assert(isFunction(cb));    
  var self = this;
  var query = 'SELECT seqNumber FROM packets WHERE src = ? AND dst = ? AND ack = 0';
  self.db.all(
    query, src, self.mailboxName,
    function(err, data) {
      var seqnums = new Array(data.length);
      for (var i = 0; i < data.length; i++) {
	seqnums[i] = data[i].seqNumber;
      }
      self.sendPacketSub(T,
	src/*back to the source*/,
	ACKLABEL,
	serializeSeqNums(seqnums),
	function(err) {
	  if (err == undefined) {
	    self.setAcked(T,
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
Mailbox.prototype.sendAckIfNeeded = function(T, src, cb) {
  assert(isIdentifier(src));
  assert(isFunction(cb));    
  var self = this;
  this.getNonAckCount(src, function(err, count) {
    if (err == undefined) {
      if (count < self.ackFrequency) {
	cb(err);
      } else {
	self.sendAck(T, src, cb);
      }
    } else {
      cb(err);
    }
  });
}

// Maximize the c-number for this mailbox as a sender
Mailbox.prototype.maximizeCNumber = function(T, dst, cb) {
  assert(isIdentifier(dst));

  // We are never sending packets to ourself, are we?
  assert(dst != this.mailboxName);
  
  assert(isFunction(cb));    
  var update = function(x) {

    self.updateCTable(T,
      self.mailboxName,
      dst,
      x,
      cb);
  };

  var src = this.mailboxName;
  
  // retrieve the first seqNumber that has not been acked.
  var query = 'SELECT seqNumber FROM packets WHERE ack = 0 AND src = ? ORDER BY seqNumber ASC';
  var self = this;
  T.get(query, src, function(err, row) {
    if (err == undefined) {
      if (row == undefined) { // No packets found, set it to 1 + the latest ack
	var query = 'SELECT seqNumber FROM packets WHERE ack = 1 AND src = ? ORDER BY seqNumber DESC';
	T.get(query, src, function(err, row) {
	  if (err == undefined) {
	    if (row == undefined) {

	      // No packets, so let the C number remain the same, whatever it was.
	      cb();
	      
	    } else {

	      // The last packet that was acked + 1, in case no packets with ack=0
	      update(bigint.inc(row.seqNumber));
	    }
	  } else {
	    cb(err);
	  }
	});
      } else {

	// The first packet not acked.
	update(row.seqNumber);
      }
    } else {
      cb(err);
    }
  });
}

Mailbox.prototype.callOnAcknowledged = function(packet, seqnums, cb) {
  if (this.onAcknowledged != undefined) {
    callHandlers(
      this,
      this.onAcknowledged,
      {
	dst: packet.src, // The mailbox we sent to
	seqnums: seqnums // The sequence numbers.
      },
      cb
    );
  } else {
    cb();
  }
}

Mailbox.prototype.handleAckPacketIfNeeded = function(T, packet, cb) {
  assert(isValidPacket(packet));
  assert(isFunction(cb));
  var self = this;
  if (packet.label == ACKLABEL && packet.dst == this.mailboxName) {
    var seqnums = deserializeSeqNums(packet.data);
    // Optional call to function whenever some packets that we sent were acknowledged.
    this.callOnAcknowledged(
      packet, seqnums,
      function (err) {
	if (err) {
	  cb(err);
	} else {
	  self.setAcked(
	    T,
	    self.mailboxName, packet.src,
	    seqnums,
	    cb
	  );
	}
      }
    );
  } else {
    cb();
  }
}


function callHandlersArray(self, handlers, data, cb) {
  if (handlers.length == 0) {
    cb();
  } else {
    handlers[0](
      self,
      data,
      function(err) {
	if (err) {
	  cb(err);
	} else {
	  callHandlersArray(self, handlers.slice(1), data, cb);
	}
      }
    );
  }
}

function callHandlers(self, handlers, data, cb) {
  if (handlers == undefined) {
    cb();
  } else if (typeof handlers == 'function') {
    handlers(self, data, cb);
  } else { // It is an array
    callHandlersArray(self, handlers, data, cb);
  }
}

Mailbox.prototype.callOnPacketReceived = function(packet, cb) {
  if (this.onPacketReceived != undefined
      && packet.dst == this.mailboxName) {
    callHandlers(this, this.onPacketReceived, packet, cb);
  } else {
    cb();
  }
  
}

// This method is called only for packets that should not be rejected.
Mailbox.prototype.acceptIncomingPacket = function(T, packet, cb) {
  assert(isValidPacket(packet));
  assert(isFunction(cb));    
  var self = this;

  // This call will
  //  * Store the packet in the mailbox
  //  * Update the C-table using the data of the packet
  this.registerPacketData(T, packet, function(err) {
    if (err == undefined) {
      self.callOnPacketReceived(
	packet,
	function(err) {
	  // If the packet was intended for this mailbox,
	  // this call will mark packets as acknowledged
	  // and maximize the C-number.
	  self.handleAckPacketIfNeeded(T,
	    packet,
	    function(err) {
	      if (err == undefined) {
		// Always maximize the C-number
		self.maximizeCNumber(T,
		  packet.src,
		  function (err) {
		    if (err == undefined) {
		      self.sendAckIfNeeded(
			T,
			packet.src, cb);
		    } else {
		      cb(err);
		    }
		  }
		);
	      } else {
		cb(err);
	      }
	    }
	  );
	}
      );
    } else {
      cb(err);
    }
  });
}

// Handle an incoming packet.
Mailbox.prototype.handleIncomingPacket = function(packet, cb) {
  assert(isValidPacket(packet));
  assert(isFunction(cb));
  var self = this;
  self.db.beginTransaction(function(err, T) {
    var cb2 = function(err, result) {
      T.commit(function(err2) {
	cb(err || err2, result);
      });
    }
    self.isAdmissibleSub(
      T,
      packet.src,
      packet.dst,
      packet.seqNumber,
      function(err, p) {
	assert(err == undefined);
	if (err == undefined) {
	  if (p) {
	    self.acceptIncomingPacket(T, packet, cb2);
	  } else {
	    cb2(err);
	  }
	} else {
	  cb2(err);
	}
      });
  });
}



Mailbox.prototype.getDiaryAndSeqNumbers = function(T, dst, cb) {
  assert(isIdentifier(dst));
  assert(isFunction(cb));    
  var self = this;
  self.makeNewDiaryNumber(T, function(err, diaryNumber) {
    if (err == undefined) {
      self.makeNewSeqNumber(T, dst, function(err, seqNumber) {
	if (err == undefined) {
	  assert(isCounter(diaryNumber));
	  assert(isCounter(seqNumber));
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
    'SELECT diaryNumber, seqNumber, ack FROM packets',
    function (err, results) {
      if (err == undefined) {
	console.log('PACKET SUMMARY OF ' + self.mailboxName + ' (' + results.length + ' packets)');
	for (var i = 0; i < results.length; i++) {
	  var r = results[i];
	  console.log('  diaryNumber = ' + r.diaryNumber +
		      '   seqNumber = ' + r.seqNumber + '   ack = ' + r.ack);
	}
	cb(err);
      } else {
	cb(err);
      }
    }
  );
}

Mailbox.prototype.reset = function(cb) {
  this.db.beginTransaction(function(err, T) {
    if (err) {
      cb(err);
    } else {
      dropTables(T, function(err) {
	if (err) {
	  cb(err);
	} else {
	  createAllTables(T, function(err) {
	    T.commit(function(err2) {
	      cb(err || err2);
	    });
	  });
	}
      });
    }
  });
}

Mailbox.prototype.close = function(cb) {
  this.db.close(cb);
}

// Given destination mailbox, label and data,
// a new packet is produced that is put in the packets table.
Mailbox.prototype.sendPacketSub = function (T, dst, label, data, cb) {
  assert(isNumber(label));
  assert(isIdentifier(dst));
  assert(isFunction(cb));
  var self = this;
  if ((typeof data != 'string') && (typeof data != 'object')) {
    cb(new Error('Please only send data in the form of a Buffer'));
  } else {
    
    if (typeof data == 'string') {
      console.log('It is recommended that the data you store is Buffer. Use string only for debugging.');
    }
    this.getDiaryAndSeqNumbers(T,
      dst,
      function(err, results) {
	if (err == undefined) {
	  var seqNumber = results.seqNumber;
	  self.getOrMakeCNumber(
	    T, 
	    dst, results.seqNumber,
	    function(err, cNumber) {
	      // Now we have all we need to make the packet.
	      var query = 'INSERT INTO packets VALUES (?, ?, ?, ?, ?, ?, ?, ?)';
	      T.run(
		query, results.diaryNumber,
		self.mailboxName, dst, results.seqNumber,
		cNumber, label, data, false,/*not yet acknowledged*/
		cb2);
	    });
	} else {
	  cb2(err);
	}
      });
  }
};

Mailbox.prototype.sendPacket = function(dst, label, data, cb) {
  var self = this;
  self.db.beginTransaction(function(err, T) {
    if (err) {
      cb(err)
    } else {
      var cb2 = function(err) {
	T.commit(function(err2) {
	  cb(err || err2);
	});
      }
      self.sendPacketSub(T, dst, label, data, cb2);
    }
  });
};
	

// Send multiple packets to the same destination
// and with the same label, but with different data, stored in an array.
//
// Convenient when we need to chop up a big file in smaller packets
// for robust transfer over e.g. bluetooth.
Mailbox.prototype.sendPackets = function(dst, label, dataArray, cb) {
  if (dataArray.length == 0) {
    cb();
  } else {
    var self = this;
    this.sendPacket(
      dst, label, dataArray[0],
      function (err) {
	if (err) {
	  cb(err);
	} else {
	  self.sendPackets(dst, label, dataArray.slice(1), cb);
	}
      }
    );
  }
}




module.exports.dispAllTableData = dispAllTableData;
module.exports.expand = expand;
module.exports.isCounter = isCounter;
module.exports.isIdentifier = isIdentifier;
module.exports.isValidMailboxName = isValidMailboxName;
module.exports.serializeSeqNums = serializeSeqNums;
module.exports.deserializeSeqNums = deserializeSeqNums;
module.exports.serializeString = serializeString;
module.exports.ACKLABEL = ACKLABEL;
module.exports.tryMakeMailbox = tryMakeMailbox;

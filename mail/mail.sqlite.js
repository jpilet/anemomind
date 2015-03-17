/*

Mailbox model based on sqlite

*/

var sqlite3 = require('sqlite3').verbose();
var seqnums = require('./seqnums.js');
var async = require('async');


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

function initializePacketsTable(db, cb) {
    initializeTableIfNotAlready(
	db, 'packets',
	['diarynumber BIGINT',
	 'src TEXT',
	 'dst TEXT',         
	 'seqnumber BIGINT',
	 'cnumber BIGINT',
	 'label TEXT', 
	 'data BLOB'], cb);
}

function initializeSeqNumbersTable(db, cb) {
    initializeTableIfNotAlready(
	db, 'seqnumbers',
	['dst TEXT', 'counter BIGINT'],
	cb
    );
}

function initializeCTable(db, cb) {
    initializeTableIfNotAlready(
	db, 'ctable',
	['src TEXT',
	 'dst TEXT',
	 'counter BIGINT'],
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
    var self = this;

    // Wait for the creation of all tables to complete before we call cb.
    async.parallel([
	function(a) {initializeSeqNumbersTable(self.db, a);},
	function(a) {initializePacketsTable(self.db, a);},
	function(a) {initializeCTable(self.db, a);},
    ], function(err) {
	cb(err);
    });
}


// Returns the current sequence number stored in the database,
// by calling a callback with that number.
// If no such number exists, it calls the callback without any arguments.
Mailbox.prototype.getCurrentSeqNumber = function(dst, callbackNewNumber) {
    if (!isNonEmptyString(dst)) {
	throw new Error('Dst should be a string');
    }
    var self = this;
    this.db.serialize(function() {
	self.db.get('SELECT counter FROM seqnumbers WHERE dst = ?', dst,
	   function(err, row) {
	       if (err == undefined) {
		   if (row == undefined) {
		       callbackNewNumber();
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
	console.log('Read number ' + x);
	if (x == undefined) {
	    var newNumber = seqnums.make();
	    console.log('New number is ' + newNumber);
	    self.db.run('INSERT INTO seqnumbers VALUES (?, ?);',
			dst, newNumber, makeCompletedFun(newNumber));
	} else {
	    var newNumber = seqnums.next(x);
	    console.log('New number is ' + newNumber);
	    self.db.run('UPDATE seqnumbers SET counter = ? WHERE dst = ?',
			newNumber, dst, makeCompletedFun(newNumber));
	}
    };
    this.getCurrentSeqNumber(dst, cbNumberRetrived);
}

// Gets the last diary number of all messages
Mailbox.prototype.getLastDiaryNumber = function(cb) {
    var query = 'SELECT max(diarynumber) FROM packets';
    this.db.get(query, function(err, result) {
	if (err == undefined) {
	    cb(err, result["max(diarynumber)"]);
	} else {
	    cb(err, null);
	}
    });
};

// Call this method whenever we send or handle a packet.
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



Mailbox.sendPacket = function (dst, label, data) {
    
};

Mailbox.prototype.rulle = function() {
    console.log('RULLE!!!');
};




console.log('Make a test mailbox');

function errThrow(err) {
    if (err != undefined) {
	throw new Error('Something wen wrong');
    }
}

var inMemory = true;
var filename = (inMemory? ':memory:' : 'demo.db');
var box = new Mailbox(filename, 'demobox', function(err) {

	['diarynumber BIGINT',
	 'src TEXT',
	 'dst TEXT',         
	 'seqnumber BIGINT',
	 'cnumber BIGINT',
	 'label TEXT', 
	 'data BLOB'];

    box.getLastDiaryNumber(function(err, num) {
	errThrow(err);
	console.log('Last diary number is %j', num);
	box.makeNewDiaryNumber(function(err, num) {
	    console.log('A new number is ' + num);
	});
    });

    if (false) {
	box.db.run('INSERT INTO packets VALUES (?, ?, ?, ?, ?, ?, ?)',
    		   129, "abra", "kadabra", 119, 109, "testpacket", "sometestdata",
    		   function(err) {
    		       errThrow(err);
    		       box.getLastDiaryNumber(function(err, num) {
    			   errThrow(err);
    			   console.log('Last diary number is %j', num);
    		       });
    		   });
    }
		
    
    if (false) {
	tableExists(box.db, 'seqnumbers');
	tableExists(box.db, 'packets');
    }
    
    if (err != undefined) {
	console.log('SOMETHING WENT WRONG WHEN BUILDING MAILBOX!!!!');
	return;
    }
    
    console.log(box.db);

    // See if we 
    if (true) {
	box.getCurrentSeqNumber('mjao', function(err, x) {
	    console.log('Current seq number for mjao is ' + x);
	});
	var dispnum = function(x) {console.log('The number is ' + x);};
    }

    // Sequence number generation
    if (true) {
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
});


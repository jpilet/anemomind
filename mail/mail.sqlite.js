/*

Mailbox model based on sqlite

*/

var sqlite3 = require('sqlite3').verbose();
var seqnums = require('./seqnums.js');


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

function tableExists(db, tableName, cb) {
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

function initializeTableIfNotAlready(db,            // <-- A sqlite3 database
				     tableName,     // <-- Name of the table to be created.
				     fieldSpecs,    // <-- The fields of the table
				     cb) {          // <-- A callback that accepts optional err.
    tableExists(db, tableName, function(status) {
	console.log('status = ' + status);
	if (status == false) {
	    db.run(makeCreateCmd(tableName, fieldSpecs), cb);
	} else if (status == true) {
	    cb();
	} else {
	    cb(status);
	}
    });
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 P A C K E T S   T A B L E

 This is a table that holds all packets.
*/

function initializePacketsTable(db, cb) {
    initializeTableIfNotAlready(
	db, 'packets',
	['diarynumber BIGINT',
	 'src TEXT',
	 'dst TEXT',         
	 'seqnumber BIGINT',
	 'label TEXT', 
	 'data BLOB'], cb);
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 S E Q C O U N T E R S 

 This is a table that holds sequence counters for every mailbox
 that we might want to sent to.
*/
function initializeSeqNumbersTable(db, cb) {
    initializeTableIfNotAlready(
	db, 'seqnumbers',
	['dst TEXT',
	 'counter BIGINT'], cb);
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

    var self = this;
    //initializePacketsTable(self.db);
    initializeSeqNumbersTable(self.db, cb);
    //initializeCTable(this.db);
}

// Returns the current sequence number stored in the database, by calling a callback with that number.
// If no such number exists, it calls the callback without any arguments.
Mailbox.prototype.getCurrentSeqNumber = function(dst, callbackNewNumber) {
    if (!isNonEmptyString(dst)) {
	throw new Error('Dst should be a string');
    }
    var self = this;
    this.db.serialize(function() {
	self.db.get('SELECT counter FROM seqnumbers WHERE dst = ?', dst,
	   function(err, row) {
	       if (row == undefined) {
		   callbackNewNumber();
	       } else {
		   callbackNewNumber(row.counter);
	       }
	   });
    });
};

// Makes a new sequence number that can be used.
Mailbox.prototype.makeNextSeqNumber = function(dst, callbackNewNumber) {
    var self = this;
    var cbNumberRetrived = function(x) {
	var makeCompletedFun = function(y) {
	    return function(err) {	
		if (err == undefined) {
		    callbackNewNumber(y);
		} else {
		    callbackNewNumber(err);
		}
	    };
	}
	if (x == undefined) {
	    var newNumber = seqnums.make();
	    self.db.run('INSERT INTO seqnumbers VALUES (?, ?);',
			dst, newNumber, makeCompletedFun(newNumber));
	} else {
	    var newNumber = seqnums.next(x);
	    self.db.run('UPDATE seqnumbers SET counter = ? WHERE dst = ?',
			newNumber, dst, makeCompletedFun(newNumber));
	}
    };
    
    this.getCurrentSeqNumber(dst, cbNumberRetrived);
}



Mailbox.sendPacket = function (dst, label, data) {
    
};

Mailbox.prototype.rulle = function() {
    console.log('RULLE!!!');
};




console.log('Make a test mailbox');

var inMemory = false;
var filename = (inMemory? ':memory:' : 'demo.db');
var box = new Mailbox(filename, 'demobox', function(err) {
    if (err != undefined) {
	console.log('SOMETHING WENT WRONG WHEN BUILDING MAILBOX!!!!');
	return;
    }
    
    console.log(box.db);

    // See if we 
    if (true) {
	box.getCurrentSeqNumber('mjao', function(x) {
	    console.log('Current seq number for mjao is ' + x);
	});
	var dispnum = function(x) {console.log('The number is ' + x);};
    }

    // Sequence number generation
    if (true) {
	box.makeNextSeqNumber('abra', function(x) {
	    console.log('First seq num is ' + x);
	    box.makeNextSeqNumber('abra', function(x) {
		console.log('Second seq num is ' + x);
		box.makeNextSeqNumber('abra', function(x) {
		    console.log('Third seq num is ' + x);
		});
	    });
	});
    }

});


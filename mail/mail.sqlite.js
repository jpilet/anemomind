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

function initializeTableIfNotAlready(db,            // <-- A sqlite3 database
				     tableName,     // <-- Name of the table to be created.
				     createTable) { // <-- Function that should create the table.
    db.get('SELECT name FROM sqlite_master WHERE type=\'table\' AND name=\''
	   + tableName + '\'', function(err, row) {
	       if (err == null) {
		   if (row == undefined) {
		       console.log('Create a new table');
		       createTable();
		   } else {
		       console.log('The table '
				   + tableName + ' already exists, no need to create it');
		   }
	       } else {
		   throw new Error('Error when checking existence of table.');
	       }
	   });
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 P A C K E T S   T A B L E

 This is a table that holds all packets.
*/

function initializePacketsTable(db) {
    var tableName = 'packets';
    initializeTableIfNotAlready(
	db, tableName,
	function() {
	    cmd = 'CREATE TABLE ' + tableName + ' ('
		   + 'diarynumber BIGINT, ' // Unique number used internally by this mailbox for every message.
		   + 'src TEXT, '           // Text string encoding sender mailbox
		   + 'dst TEXT, '           // Text string encoding receiver mailbox
		   + 'seqnumber BIGINT, '   // Sequence number. Value of a counter increased by 1 for every packet sent.
		   + 'label TEXT, '         // Description of what the packet contains
		   + 'data BLOB'            // The data to be sent. Can be in any form.
		   + ');';
	    runWithLog(db, cmd);
	});
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 S E Q C O U N T E R S 

 This is a table that holds sequence counters for every mailbox
 that we might want to sent to.
*/
function initializeSeqCountersTable(db) {
    var tableName = 'seqcounters';
    initializeTableIfNotAlready(
	db, tableName,
	function() {
	    cmd = 'CREATE TABLE ' + tableName + ' ('
		+ 'dst TEXT, '
		+ 'counter BIGINT'
		+ ');';
	    runWithLog(db, cmd);
	});
    
}


// A constructor for a temprorary storage of all mails.
function Mailbox(dbFilename,      // <-- The filename where all
		                  //     messages are stored.
		 thisMailboxName, // <-- A string that uniquely
		                  //     identifies this mailbox
		 errorCallback) { // <-- Optional error callback for db.
    if (!isValidDBFilename(dbFilename)) {
	throw new Error('Invalid database filename');
    }
    if (!isValidMailboxName(thisMailboxName)) {
	throw new Error('Invalid mailbox name');
    }
    this.dbFilename = dbFilename;
    this.mailboxName = thisMailboxName;
    this.db = new sqlite3.Database(dbFilename, errorCallback);

    // Tables that don't exist should be created.
    initializePacketsTable(this.db);
    initializeSeqCountersTable(this.db);
    //initializeCTable(this.db);
}

// Returns the current sequence number stored in the database, by calling a callback with that number.
// If no such number exists, it calls the callback without any arguments.
Mailbox.prototype.getCurrentSeqNumber = function(dst, callbackNewNumber) {
    if (!isNonEmptyString(dst)) {
	throw new Error('Dst should be a string');
    }
    this.db.run('SELECT FROM seqnumbers WHERE dst = ?;', dst,
	   function(err, row) {
	       if (row == undefined) {
		   callbackNewNumber();
	       } else {
		   callbackNewNumber(row.counter);
	       }
	   });
};

Mailbox.prototype.getCurrentSeqNumber2 = function(dst, cb) {
    this.getCurrentSeqNumber(dst, cb);
};

// Makes a new sequence nubmer that can be used
Mailbox.prototype.makeNextSeqNumber = function(dst, callbackNewNumber) {
    var self = this;
    var cbNumberRetrived = function(x) {
	var newNumber = undefined;
	var makeCompletedFun = function(y) {
	    return function(err) {	
		if (err == undefined) {
		    callbackNewNumber(y);
		} else {
		    callbackNewNumber();
		}
	    };
	}
	if (x == undefined) {
	    var newNumber = seqnums.make();
	    console.log('INSERT ' + newNumber);
	    self.db.run('INSERT INTO seqcounters VALUES (?, ?);',
			dst, newNumber, makeCompletedFun(newNumber));
	} else {
	    console.log('UPDATE' + newNumber);
	    var newNumber = seqnums.next(x);
	    self.db.run('UPDATE seqcounters SET counter = ? WHERE dst = ?',
			newNumber, dst, makeCompletedFun(newNumber));
	}
    };
    
    this.getCurrentSeqNumber(dst, cbNumberRetrived);//function(x) {});
}



Mailbox.sendPacket = function (dst, label, data) {
    
};

Mailbox.prototype.rulle = function() {
    console.log('RULLE!!!');
};




console.log('Make a test mailbox');
var box = new Mailbox('demo.db', 'demobox');
//console.log(typeof box.db);
console.log(box.db);

if (false) {
    console.log('Created');
    box.getCurrentSeqNumber('mjao', function(x) {
	console.log('Current seq number for mjao is ' + x);
    });
    var dispnum = function(x) {console.log('The number is ' + x);};
}

box.makeNextSeqNumber('abra', function(x) {
    console.log('First seq num is ' + x);
    box.makeNextSeqNumber('abra', function(x) {
	console.log('Second seq num is ' + x);
    });
});

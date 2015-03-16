var sqlite3 = require('sqlite3').verbose();

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
		   + 'diarynumber TEXT, ' // Unique number used internally by this mailbox for every message.
		   + 'src TEXT, '         // Text string encoding sender mailbox
		   + 'dst TEXT, '         // Text string encoding receiver mailbox
		   + 'seqnumber TEXT, '   // Sequence number. Value of a counter increased by 1 for every packet sent.
		   + 'label TEXT, '       // Description of what the packet contains
		   + 'data BLOB'          // The data to be sent. Can be in any form.
		   + ');';
	    runWithLog(db, cmd);
	});
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 S E Q C O U N T E R S 

 This is a table that holds sequence counters.
*/
function initializeSeqCountersTable(db) {
    var tableName = 'seqcounters';
    initializeTableIfNotAlready(
	db, tableName,
	function() {
	    cmd = 'CREATE TABLE ' + tableName + ' ('
		+ 'dst TEXT'
		+ 'counter TEXT'
		+ ');';
	    runWithLog(db, cmd);
	});
    
}

// A complete packet.
function Packet(src, dst, seqnumber, label, data) {
    this.src = src;
    this.dst = dst;
    this.seqnumber = seqnumber;
    this.label = label;
    this.data = data;
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

// Every message in this mailbox is a assigned a unique so called 'diary number'.
Mailbox.makeNewDiaryNumber = function(dst) {
    
}

// Returns the current sequence number by calling a callback.
Mailbox.getCurrentSeqNumber = function(dst, callbackNewNumber) {
    if (!isNonEmptyString(dst)) {
	throw new Error('Dst should be a string');
    }
    db.run('SELECT FROM seqnumbers WHERE dst = \'' + dst + '\';',
	   function(err, row) {
	       if (row == undefined) {
		   callbackNewNumber();
	       } else {
		   callbackNewNumber(row.counter);
	       }
	   });
};



Mailbox.sendPacket = function (dst, label, data) {
    
};

Mailbox.prototype.rulle = function() {
    console.log('RULLE!!!');
};




console.log('Make a test mailbox');
var box = new Mailbox('demo.db', 'demobox');
//console.log(typeof box.db);
console.log(box.db);
console.log('Created');
box.rulle();

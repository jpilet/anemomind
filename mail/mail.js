var sqlite3 = require('sqlite3').verbose();

function isValidDBFilename(x) {
    if (typeof x == 'string') {
	return true;
    }
    return false;
}

function isValidMailboxName(x) {
    if (typeof x == 'string') {
	if (x.length > 0) {
	    return true;
	}
    }
    return false;
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
		   + 'sender TEXT, '        // Text string encoding sender mailbox
		   + 'receiver TEXT, '          // Text string encoding receiver mailbox
		   + 'seqnumber TEXT, '    // Sequence number. Value of a counter increased by 1 for every packet sent.
		   + 'label TEXT, '        // Description of what the packet contains
		   + 'data BLOB'          // The data to be sent. Can be in any form.
		   + ');';
	    console.log('Run ' + cmd);
	    db.run(cmd);
	});
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
 C T A B L E

 This is a table that holds all C-numbers seen so far.

*/
function initializeCTable(db) {
    var tableName = 'ctable';
    initializeTableIfNotAlready(
	db, tableName,
	function() {
	    cmd = 'CREATE TABLE ' + tableName + ' ('
		+ 'sender TEXT, '
		+ 'receiver TEXT, '
		+ 'cnumber TEXT'
		+ ');';
	    console.log('Run ' + cmd);
	    db.run(cmd, function(err) {
		
	    });
	}
    );
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
    initializePacketsTable(this.db);
    initializeCTable(this.db);
}

Mailbox.prototype.rulle = function() {
    console.log('RULLE!!!');
}




console.log('Make a test mailbox');
var box = new Mailbox('demo.db', 'demobox');
//console.log(typeof box.db);
console.log(box.db);
console.log('Created');
box.rulle();

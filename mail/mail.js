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
}




console.log('Make a test mailbox');
var box = new Mailbox('demo.db', 'demobox');
console.log('Created');

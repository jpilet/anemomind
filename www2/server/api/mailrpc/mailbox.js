var mb = require('../../components/mail/mail.sqlite.js');



function openMailbox(mailboxName, cb) {
    if (!mb.isValidMailboxName(mailboxName)) {
	cb(new Error('Invalid mailbox name: ' + mailboxName));
    } else {
	var filename = mailboxName + '.mailsqlite.db';
	mb.tryMakeMailbox(
	    filename, mailboxName, cb
	);
    }
}

exports.openMailbox = openMailbox;

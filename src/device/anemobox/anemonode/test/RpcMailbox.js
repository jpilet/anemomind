var builder = require('../components/RpcMailboxBuilder.js');
var assert = require('assert');

var rpcTable = {};
builder.fillTable(rpcTable);

var mailboxName = 'rulle';

function prepareMailbox(cb) {
    rpcTable.mb_reset({
	thisMailboxName: mailboxName,
    }, function(response) {
	assert(response.error == undefined);
	cb(response);
    });
}

describe(
    'reset',
    function() {
	it(
	    'Should reset the mailbox and make sure the packet count is 0',
	    function(done) {
		prepareMailbox(function(response) {
		    rpcTable.mb_getTotalPacketCount({
			thisMailboxName: mailboxName
		    }, function(response) {
			assert(response.error == undefined);
			assert(response.result == 0);
			done();
		    });
		});
	    }
	);
    }
);

var builder = require('../components/RpcMailbox.js');
var mb = require('../components/LocalMailbox.js');
var assert = require('assert');

var rpcTable = {};
builder.fillTable(rpcTable);

var mailboxName = 'rulle';

function prepareMailbox(cb) {
  rpcTable.mb_reset({
    thisMailboxName: mailboxName,
  }, function(response) {
    assert.equal(response.error, undefined);
    cb(response);
  });
}

describe(
  'reset',
  function() {
    mb.setMailRoot('/tmp/anemobox/');
    it(
      'Should reset the mailbox and make sure the packet count is 0',
      function(done) {
	prepareMailbox(function(response) {
	  rpcTable.mb_getTotalPacketCount({
	    thisMailboxName: mailboxName
	  }, function(response) {
	    assert.equal(response.error, undefined);
	    assert.equal(response.result, 0);
	    done();
	  });
	});
      }
    );
  }
);

describe(
  'getForeignDiaryNumber',
  function() {
    it(
      'Should get a foreign diary number from an empty mailbox',
      function(done) {
	mb.setMailRoot('/tmp/anemobox/');
	prepareMailbox(function(response) {
	  assert.equal(response.error, undefined);
	  rpcTable.mb_getForeignDiaryNumber({
	    thisMailboxName: mailboxName,
	    otherMailbox: "evian"
	  }, function(response) {
	    assert.equal(response.result, undefined);
	    rpcTable.mb_setForeignDiaryNumber({
	      thisMailboxName: mailboxName,
	      otherMailbox: "evian",
	      newValue: "0000000000000009"
	    }, function(response) {
	      assert.equal(response.error, undefined);
	      rpcTable.mb_getForeignDiaryNumber({
		thisMailboxName: mailboxName,
		otherMailbox: "evian"
	      }, function(response) {
		assert.equal(response.result, "0000000000000009");
		done();
	      });
	    });
	  });
	});
      }
    );
  }
);

describe(
  'error',
  function() {
    it(
      'Should result in an error',
      function(done) {
	mb.setMailRoot('/tmp/anemobox/');
	rpcTable.mb_reset({
	  // Omit 'thisMailboxName'
	}, function(response) {
	  assert(response.error);
	  done();
	})
      }
    );
  }
);

describe(
  'error2',
  function() {
    it(
      'Should result in an error',
      function(done) {
	mb.setMailRoot('/tmp/anemobox/');
	rpcTable.mb_getForeignDiaryNumber({
	  thisMailboxName: mailboxName
	  // omit the required argument for the function
	}, function(response) {
	  assert(response.error);
	  done();
	})
      }
    );
  }
);

describe('mb_has', function() {
  it(
    'Check that it is false and doesnt crash',
    function(done) {
      mb.getName(function(name) {
        var strangeName = 'somestrangename123123123123123';
        assert(strangeName != name);
	rpcTable.mb_has({
	  name: strangeName
	}, function(response) {
          console.log(response);
	  assert(response.result === false);
	  done();
	});
      });
    });
});

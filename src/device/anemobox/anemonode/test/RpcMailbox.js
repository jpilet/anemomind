var builder = require('../components/RpcMailbox.js');
var mb = require('../components/LocalMailbox.js');
var assert = require('assert');
var common = require('mail/common.js');
var files = require('mail/files.js');
var fs = require('fs');
var rpcTable = {};
var config = require('../components/config.js');
var Q = require('q');
var msgpack = require('msgpack-js');
builder.fillTable(rpcTable);

var mailboxName = null;

function withErr(f) {
  return function(x, cb) {
    f(x, function(result) {
      cb(null, result);
    });
  }
}

function prepareMailbox(cb) {
  mb.getName(function(lname) {
    mailboxName = lname;
    rpcTable.ep_reset({
      name: lname,
    }, function(response) {
      assert.equal(response.error, undefined);
      cb(response);
    });
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
	  rpcTable.ep_getTotalPacketCount({
	    name: mailboxName
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
  'updateLowerBounds',
  function() {
    it(
      'Should get the src dst pairs',
      function(done) {
	mb.setMailRoot('/tmp/anemobox/');
	prepareMailbox(function(response) {
	  assert.equal(response.error, undefined);
          assert(typeof mailboxName == 'string');
	  rpcTable.ep_updateLowerBounds({
	    name: mailboxName,
	    pairs: [{src:'a', dst:'b'}],
	  }, function(response) {
	    assert.equal(response.result[0], "0000000000000000");
	    rpcTable.ep_updateLowerBounds({
	      name: mailboxName,
              pairs: [{src: 'a',
                       dst: 'b',
	               lb: "0000000000000009"}]
	    }, function(response) {
	      assert.equal(response.error, undefined);
	      assert.equal(response.result[0], "0000000000000009");
	      done();
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
	rpcTable.ep_reset({
	  // Omit 'name' to provoke an error.
	}, function(response) {
	  assert(response.error);
	  done();
	})
      }
    );
  }
);

describe('files', function() {
  it('f2', function(done) {
    this.timeout(12000);
    process.env.ANEMOBOX_CONFIG_PATH = '/tmp/anemoboxcfg';
    assert(config.getConfigPath() == '/tmp/anemoboxcfg');
    mb.setMailRoot('/tmp/anemobox/');
    prepareMailbox(function(response) {
      assert(typeof mailboxName == 'string');
      var srcFilename = '/tmp/srcboat.dat';
      Q.nfcall(fs.writeFile, srcFilename, 'Some data for the box')
        .then(function() {
          return files.packFiles([{src: srcFilename, dst: 'boat.dat'}])
        })
        .then(function(packed) {
          return Q.nfcall(
            withErr(rpcTable.ep_putPacket), {
              name: mailboxName,
              packet: {
                src: 'boat119',
                label: common.files,
                seqNumber: "0000000000000111",
                data: msgpack.encode(packed),
                dst: mailboxName
              }
            });
        }).then(function(response) {
          setTimeout(function() { // <-- Wait a bit so that the packet handler has time to save the file. A bit dirty.
            Q.all([
              Q.nfcall(fs.readFile, '/tmp/srcboat.dat'),
              Q.nfcall(fs.readFile, '/tmp/anemoboxcfg/boat.dat')
            ]).then(function(fdata) {
              fdata[0].equals(fdata[1]);
              done();
            }).catch(function(e) {
              console.log('In testcase f2, got error');
              console.log(e);
              done(e);
            });
          }, 500);
        });
    });
  });
});


describe(
  'error2',
  function() {
    it(
      'Should result in an error',
      function(done) {
	mb.setMailRoot('/tmp/anemobox/');

        prepareMailbox(function(response) {
	  rpcTable.ep_updateLowerBounds({
	    name: mailboxName
	    // omit the required argument for the function
	  }, function(response) {
	    assert(response.error);
	    done();
	  })
        });
      }
    );
  }
);

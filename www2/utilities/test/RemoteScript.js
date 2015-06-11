var testPath = '/tmp/mailboxes/boat123456789012345678901234.mailsqlite.db';
var assert = require('assert');
var naming = require('mail/naming.js');
var Boat = require('../../server/api/boat/boat.model.js');
var common = require('../common.js');
var path = require('path');
var script = require('mail/script.js');
var mb = require('mail/mail.sqlite.js');

function withTestBoat(cbOperation, cbDone) {
  Boat.create({
    name: 'Frida',
    type: 'IF',
    sailNumber: '1604',
    anemobox: 'abc119'}, function(err, docInserted) {
      var id = docInserted._id;
      cbOperation(id, function(err) {
        Boat.remove({_id: id}, function(err2) {
          cbDone(err || err2);
        });
      });
    });
}

function withConnectionAndTestBoat(cbOperation, cb) {
  common.withMongoConnection(function(ref) {
    withTestBoat(cbOperation, cb);
  });
}

//function withConnectionAndBoat(cbOperation, cbDone)

describe('RemoteScript', function() {
  it('Should parse the filename of mailbox', function() {
    var parsed = common.extractBoatIdFromFilename(testPath);
    assert(parsed == '123456789012345678901234');
  });

  // Just so that we can perform test cases.
  it('Should insert and remove a test boat. Check that there is an id', function(done) {
    withConnectionAndTestBoat(function(id, cb) {
      assert(id);
      assert(id.constructor.name == 'ObjectID');
      cb();
    }, done);
  });
  
  it('Should extract a box id', function(done) {
    withConnectionAndTestBoat(function(id, cb) {
      assert(id);
      common.getBoxIdFromBoatId(id, function(err, boxId) {
        assert(!err);
        assert.equal(boxId, 'abc119');
        cb();
      });
    }, done);
  });

  it('Should send a script for execution', function(done) {
    withConnectionAndTestBoat(function(id, cb) {
      var filename = path.join(
        '/tmp/', naming.makeDBFilename(naming.makeMailboxNameFromBoatId(id)));


      var performSync = null;
      var boxMailboxName = naming.makeMailboxNameFromBoxId('abc119');
      mb.tryMakeMailbox(
        path.join('/tmp/', naming.makeDBFilename(boxMailboxName)),
        boxMailboxName, function(err, boxMailbox) {
          assert(!err);
          assert(boxMailbox);
          boxMailbox.onPacketReceived = script.makeScriptRequestHandler(performSync);
          
          common.sendScriptToBox(filename, 'sh', 'cd /tmp\npwd', function(err, data) {
            assert(!err);
            
            done();
          });
        });
    });
  });
});




var testPath = '/tmp/mailboxes/boat123456789012345678901234.mailsqlite.db';
var assert = require('assert');
var naming = require('mail/naming.js');
var Boat = require('../server/api/boat/boat.model.js');
var BoxExec = require('../server/api/boxexec/boxexec.model.js');
var makeScriptResponseHandler = require('../server/api/boxexec/response-handler.js');
var common = require('../utilities/RemoteScriptCommon.js');
var path = require('path');
var script = require('mail/script.js');
var mb = require('mail/mail2.sqlite.js');
var sync = require('mail/sync2.js');

var removeBoat = true;
common.init();


function withTestBoat(cbOperation, cbDone) {
  Boat.create({
    name: 'Frida',
    type: 'IF',
    sailNumber: '1604',
    anemobox: 'abc119'}, function(err, docInserted) {
      var id = docInserted._id;
      cbOperation(id, function(err) {
        if (removeBoat) {
          Boat.remove({_id: id}, function(err2) {
            cbDone(err || err2);
          });
        } else {
          cbDone(err);
        }
      });
    });
}

function withConnectionAndTestBoat(cbOperation, cb) {
  common.withMongoConnection(function(ref) {
    withTestBoat(cbOperation, cb);
  });
}

//function withConnectionAndBoat(cbOperation, cbDone)

function makeAndResetMailbox(filename, mailboxName, cb) {
  mb.tryMakeEndPoint(filename, mailboxName, function(err, mailbox) {
    if (err) {
      cb(err);
    } else {
      mailbox.reset(function(err2) {
        if (err2) {
          cb(err2);
        } else {
          cb(null, mailbox);
        }
      });
    }
  });
}

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
    var scriptData = 'cd /tmp\npwd';
    withConnectionAndTestBoat(function(id, doneAll) {
      var boatId = id;
      var boatMailboxName = naming.makeMailboxNameFromBoatId(id);
      var filename = path.join(
        '/tmp/', naming.makeDBFilename(boatMailboxName));

      
      // To be set later in the code.
      var performSync = null;
      var boxMailboxName = naming.makeMailboxNameFromBoxId('abc119');
      
      // Make a mailbox for the anemobox
      makeAndResetMailbox(
        path.join('/tmp/', naming.makeDBFilename(boxMailboxName)),
        boxMailboxName, function(err, boxMailbox) {
          assert(!err);
          assert(boxMailbox);

          // Make a mailbox for the boat
          makeAndResetMailbox(filename, boatMailboxName, function(err, boatMailbox) {
            assert(!err);

            // Called when the response of executing the script is coming back.
            boatMailbox.addPacketHandler(makeScriptResponseHandler(
              function(err, response) {
                assert(!err);
                assert(response);
                assert(response._id);
                assert.equal(response.stdout, '/tmp\n');
                assert.equal(response.boxId, 'abc119');
                assert.equal(response.boatId, boatId);
                assert.equal(response.err, null);
                assert.equal(response.stderr, null);
                assert.equal(response.type, 'sh');
                assert.equal(response.script, scriptData);
                doneAll();
              }));

            // Send the script
            common.sendScriptToBox(filename, 'sh', scriptData, function(err, data) {
              assert(!err);

              performSync = function(cb) {
                sync.synchronize(boatMailbox, boxMailbox, cb);
              };
              
              boxMailbox.addPacketHandler(script.makeScriptRequestHandler(performSync));

              // Run the first sync. This will propagate the script to the box,
              // that will execute it.
              performSync(function(err) {
                assert(!err);
              });
            });
          });
        });
    }, function(err) {
      assert(!err);
      done();
    });
  });
});




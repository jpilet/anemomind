var testPath = '/tmp/mailboxes/boat123456789012345678901234.mailsqlite.db';
var assert = require('assert');
var Boat = require('../../server/api/boat/boat.model.js');
var common = require('../common.js');

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

function withConnectionAndTestBoat(cbOperation, cbDoneAll) {
  common.withMongoConnection(function(ref, cbDoneConnection) {
    withTestBoat(cbOperation, cbDoneConnection);
  }, cbDoneAll);
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
});




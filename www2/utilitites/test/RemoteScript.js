var testPath = '/tmp/mailboxes/boat123456789012345678901234.mailsqlite.db';
var assert = require('assert');


var common = require('../common.js');

describe('RemoteScript', function() {
  it('Should parse the filename of mailbox', function() {
    var parsed = common.extractBoatIdFromFilename(testPath);
    assert(parsed == '123456789012345678901234');
  });
});


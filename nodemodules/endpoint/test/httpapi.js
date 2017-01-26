var testserver = require('./testserver.js');
var assert = require('assert');

describe('app', function() {
  it('should export an app', function() {
    assert(testserver.app);
  });
});

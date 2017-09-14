var utils = require('../client/app/utils.js');
var assert = require('assert');

describe('utils', function() {
  it('setIn', function() {
    var x = utils.setIn(null, ['a', 'b', 'c'], 119);
    assert(x.a.b.c == 119);
  });
});

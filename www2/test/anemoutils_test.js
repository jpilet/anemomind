var utils = require('../client/app/anemoutils.js');
var assert = require('assert');

describe('utils', function() {
  it('setIn', function() {
    var x = utils.setIn(null, ['a', 'b', 'c'], 119);
    assert(x.a.b.c == 119);

    var y = utils.setIn({a: {b: {c: 44, d:130}}}, ['a', 'b', 'c'], 120);
    assert(y.a.b.c == 120);
    assert(y.a.b.d == 130);
  });

  it('getIn', function() {
    assert(utils.getIn(null, ['a', 'b']) == null);
    assert(utils.getIn({a: {b: 119}}, ['a', 'b']) == 119);
  });

  it('updateIn', function() {
    function inc(x) {
      return 1 + (x || 0);
    }
    var x = utils.updateIn(null, ['a', 'b'], inc);
    assert(x.a.b == 1);
    utils.updateIn(x, ['a', 'b'], inc);
    assert(x.a.b == 2);
  });
});

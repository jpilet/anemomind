var common = require('../common.js');
var assert = require('assert');

describe('isObjectWithFields', function() {
  it('Should test various objects', function() {
    assert(common.isObjectWithFields({a: 1, b:2}, ['a', 'b']));
    assert(common.isObjectWithFields({a: 1, b:2, c:3}, ['a', 'b']));
    assert(!common.isObjectWithFields({a: 1, b:2}, ['a', 'c']));
    assert(!common.isObjectWithFields(34, ['a', 'c']));
  });
});

var utils = require('../components/utils.js');
var assert = require('assert');

describe('utils', function() {
  it('unit', function() {
    assert(Math.abs(
      0.51444 - utils.taggedToSI([1.0, 'kn'])) 
           < 0.01);
  });
});

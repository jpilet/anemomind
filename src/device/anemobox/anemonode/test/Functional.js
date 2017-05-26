var assert = require('assert');
var f = require('../components/Functional.js');

function odd(x) {
  return x % 2 == 1;
}

function square(x) {
  return x*x;
}

function addToArray(dst, x) {
  dst.push(x);
  return dst;
}

function add3(x) {
  return x + 3;
}

var input = [1, 2, 3, 4, 5, 3, 2];

describe('Function', function() {
  it('map', function() {
    var output = input.reduce(f.map(square)(addToArray), []);
    assert(output.length == input.length);
    for (var i = 0; i < output.length; i++) {
      var k = input[i];
      assert(k*k == output[i]);
    }
  });

  it('filter', function() {
    var output = input.reduce(f.filter(odd)(addToArray), []);
    var expected = [1, 3, 5, 3];
    assert(output.length == expected.length);
    for (var i = 0; i < output.length; i++) {
      assert(output[i] == expected[i]);
    }
  });

  it('compose', function() {
    var g = f.compose(add3, square, add3);
    assert(g(2) == 28);
  });
});

var su = require('../components/StringUtils.js');
var assert = require('assert');

function addToArray(dst, x) {
  dst.push(x);
  return dst;
}

describe('StringUtils', function() {
  it('catSplit', function() {
    var fragments = ["aaa\nbb", "\nccc\nddd\neee", "ee\nff\n", "ggg"];
    var lines = fragments.reduce(su.catSplit('\n')(addToArray), []);

    // No 'ggg' in expected, it is still in the memory.
    // But since we are assuming an infinite stream of data,
    // we don't care for now.
    var expected = ["aaa","bb","ccc","ddd","eeeee","ff"];

    assert(lines.length == expected.length);
    for (var i = 0; i < lines.length; i++) {
      assert(lines[i] == expected[i]);
    }
  });
});

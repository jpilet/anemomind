var utils = require('../utils.js');
var im = require('immutable');
var assert = require('assert');

function testFunc(xyz, cb) {}

describe('Utils', function() {
  it('args', function() {
    var args = utils.getParamNames(testFunc);
    console.log("args = %j", args);
    assert(utils.isAsyncFunction(testFunc));
  });

  it('asyncred', function(done) {
    var src = im.List([1, 2, 3, 4, 5]);
    utils.asyncReduce(function(sum, x, cb) {
      cb(null, sum + x);
    }, 1000, src, function(err, result) {
      assert(result == 1015);
      done(err);
    });
  });

  it('asycnmap', function(done) {
    var src = im.List([1, 2, 3]);
    utils.asyncMap(
      function(x, cb) {cb(null, x + 1000);}, 
      src, function(err, result) {
        console.log("Result = %j", result);
        assert(result.equals(im.List([1001, 1002, 1003])));
        done(err);
      });
  });

  it('comp2', function(done) {
    function f(x) {return x + 3;}
    function g(x) {return 1000*x;}
    var fg = utils.comp(f, g);
    fg(7, function(err, result) {
      assert(result ==7003);
      done(err);
    });
  });
});

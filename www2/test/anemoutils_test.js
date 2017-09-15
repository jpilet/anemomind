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

  it('valueState', function() {

    var counter = 0;

    function add(x, y) {
      counter += 1;
      return x + y;
    }

    var a = new utils.ValueState();
    a.set(3);

    var b = new utils.ValueState();
    b.set(4);

    var c = new utils.ValueState(add, [a, b]);

    assert(counter == 0);
    assert(c.get() == 7);
    assert(counter == 1);

    assert(c.get() == 7);
    assert(counter == 1); // no re-evaluation. Used cached value.

    assert(c.get() == 7);
    assert(counter == 1);

    a.set(10);
    
    assert(c.get() == 14);
    assert(counter == 2);


    var d = new utils.ValueState(add, [c, a]);
    assert(d.get() == 24);
    assert(counter == 3);

    assert(d.get() == 24);
    assert(counter == 3);
    
    c.set(9);
    assert(d.get() == 19);
    assert(counter == 4);

    a.set(100);
    assert(d.get() == 204);
    assert(counter == 6);

    a.update(function(x) {return x + 1;});

    assert(d.get() == 206);
    assert(counter == 8);
  });

  it('sort by key', function() {
    var x = [{a: 9, b: -100}, {a: 3, b: 1000}, {a: 5, b: 500}];
    
    var xa = x.sort(utils.compareByKey("a"));
    assert(xa[0].a = 3);
    assert(xa[1].a = 5);
    assert(xa[2].a = 9);
    
    var xb = x.sort(utils.compareByValue(function(x) {return x.b;}, 'descend'));
    assert(xb[2].b = -100);
    assert(xb[1].b = 500);
    assert(xb[0].b = 1000);
    
  });
});

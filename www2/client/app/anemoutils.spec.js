describe('utils', function() {
  var utils = anemoutils;

  it('setIn', function() {
    var x = utils.setIn(null, ['a', 'b', 'c'], 119);
    expect(x.a.b.c).toBe(119);

    var y = utils.setIn({a: {b: {c: 44, d:130}}}, ['a', 'b', 'c'], 120);
    expect(y.a.b.c).toBe(120);
    expect(y.a.b.d).toBe(130);
  });

  it('getIn', function() {
    expect(utils.getIn(null, ['a', 'b'])).toBe(null);
    expect(utils.getIn({a: {b: 119}}, ['a', 'b'])).toBe(119);
  });

  it('updateIn', function() {
    function inc(x) {
      return 1 + (x || 0);
    }
    var x = utils.updateIn(null, ['a', 'b'], inc);
    expect(x.a.b).toBe(1);
    utils.updateIn(x, ['a', 'b'], inc);
    expect(x.a.b).toBe(2);
  });

  it('valueState', function() {

    var counter = 0;

    function add(x, y) {
      counter += 1;
      return x + y;
    }

    var a = new utils.ValueState({init: 3});

    var b = new utils.ValueState({init: 4});

    var c = new utils.ValueState({f: add, args: [a, b]});

    expect(counter).toBe(0);
    expect(c.get()).toBe(7);
    expect(counter).toBe(1);

    expect(c.get()).toBe(7);
    expect(counter).toBe(1); // no re-evaluation. Used cached value.

    expect(c.get()).toBe(7);
    expect(counter).toBe(1);

    a.set(10);
    
    expect(c.get()).toBe(14);
    expect(counter).toBe(2);


    var d = new utils.ValueState({f: add, args: [c, a]});
    expect(d.get()).toBe(24);
    expect(counter).toBe(3);

    expect(d.get()).toBe(24);
    expect(counter).toBe(3);
    
    c.set(9);
    expect(d.get()).toBe(19);
    expect(counter).toBe(4);

    a.set(100);
    expect(d.get()).toBe(204);
    expect(counter).toBe(6);

    a.update(function(x) {return x + 1;});

    expect(d.get()).toBe(206);
    expect(counter).toBe(8);
  });
});

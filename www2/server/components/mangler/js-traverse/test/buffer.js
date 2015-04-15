var test = require('tape');
var traverse = require('../');

test('bufferEach', function (t) {
    var obj = { x : new Buffer('abc'), y : 10, z : 5 };

    var counts = {};

    traverse(obj).forEach(function (node) {
        var t = (node instanceof Buffer && 'Buffer') || typeof node;
        counts[t] = (counts[t] || 0) + 1;
    });

    t.same(counts, {
        object : 1,
        Buffer : 1,
        number : 2,
    });
    t.end();
});

test('bufferMap', function (t) {
    var obj = { x : new Buffer('abc'), y : 10, z : 5 };

    var res = traverse(obj).map(function (node) {
        if (typeof node === 'number') this.update(node + 100);
    });

    t.ok(obj.x !== res.x);
    t.same(res, {
        x : obj.x,
        y : 110,
        z : 105,
    });
    t.end();
});


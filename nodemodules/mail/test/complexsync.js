var mail2 = require('../mail2.sqlite.js');
var assert = require('assert');
var schema = require('../endpoint-schema.js');
var dd = require('dentdoche');
var bigint = require('../bigint.js');
var pprint = require('dentdoche/pprint.js');
var synchronize = dd.makeAsync(require('../sync2.js').synchronize);

var pairs = [{src: "box119", dst: "boat119"}];

var received = [];
var handler = function(endPoint, packet) {
  received.push(packet.label);
}

eval(dd.parse('(dafn makeEndPoints () (map (afn (fnameAndName) (let ((fname name) fnameAndName) (acall mail2.tryMakeAndResetEndPoint (+ "/tmp/testep_" fname ".db") name))) (quote (("a" "box119") ("b" "gateway") ("c" "gateway") ("d" "boat119")))))'));

// Bind the elements of the array to named local variables.
var withEps = dd.macro(function() {
  var args = dd.argsToArray(arguments);
  var result = ['let', [['box', 'g0', 'g1', 'boat'], args[0]]].concat(args.slice(1));
  return result;
});

var myAssertSub = dd.setAsync(function(label, value, cb) {
  if (value) {
    cb(null, value);
  } else {
    cb(new Error('Assertion ' + label + ' failed.'));
  }
});

var myAssert = dd.macro(function(x) {
  return [myAssertSub, pprint.str(x), x];
});

// Transfer a packet via the first gateway
eval(dd.parse('(dafn test1 (eps) (withEps eps (let (lb0 (acall .getLowerBounds boat pairs)) (acall .sendPacket box "boat119" 139 (new Buffer (array 0 1 2 3 4))) (synchronize box g0) (myAssert (= 1 (acall .getTotalPacketCount g0))) (myAssert (= 0 (acall .getTotalPacketCount boat))) (synchronize g0 boat) (myAssert (= 0 (acall .getTotalPacketCount boat))) (myAssert (< (get lb0 0) (get (acall .getLowerBounds boat pairs) 0))))))'));

// Transfer a packet via the second gateway
eval(dd.parse('(dafn test2 (eps) (withEps eps (let (lb0 (first (acall .getLowerBounds boat pairs))) (acall .sendPacket box "boat119" 140 (new Buffer (array 3 4 5))) (synchronize box g1) (myAssert (= 2 (acall .getTotalPacketCount box))) (synchronize g1 boat)  (= (bigint.inc lb0) (first (acall .getLowerBounds boat pairs))))))'));

// Transfer the two packets and check that they were received.
eval(dd.parse('(dafn runTest () (let (eps (makeEndPoints)) (withEps eps (.addPacketHandler boat handler) (test1 eps) (test2 eps) (myAssert (= (quote (139 140)) received)))))'));// (myAssert (= 2 (acall .getTotalPacketCount box))))))'));

describe('complex', function() {
  it('Should synchronize in a complex fashion', function(done) {
    runTest(done);
  });
});

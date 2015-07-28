var mail2 = require('../mail2.sqlite.js');
var assert = require('assert');
var synchronize = require('../sync2.js').synchronize;
var schema = require('../endpoint-schema.js');
var dd = require('dentdoche');
var bigint = require('../bigint.js');
var pprint = require('dentdoche/pprint.js');

dd.declareAsync(mail2.tryMakeAndResetEndPoint, synchronize);
dd.declareAsyncMethods(mail2.EndPoint, 'sendPacket', 'getTotalPacketCount', 'getLowerBounds');

var pair = {src: "box119", dst: "boat119"};
var pairs = [pair];

var received = [];
var handler = function(endPoint, packet) {
  received.push(packet.label);
}

eval(dd.parse('(dafn makeEndPoints () (map (afn (fname name) ' +
              '(mail2.tryMakeAndResetEndPoint (+ "/tmp/testep_" fname ".db") name))' +
              '(quote ("a" "b" "c" "d")) ' +
              '(quote ("box119" "gateway" "gateway" "boat119"))))'));

// Bind the elements of the array to named local variables.
var withEps = dd.macro(function() {
  var args = dd.argsToArray(arguments);
  var result = ['let', [['box', 'g0', 'g1', 'boat'], args[0]]].concat(args.slice(1));
  return result;
});

// Transfer a packet via the first gateway
eval(dd.parse('(dafn test1 (eps) (withEps eps (let (lb0 (.getLowerBounds boat pairs)) (.sendPacket box "boat119" 139 (new Buffer (array 0 1 2 3 4))) (synchronize box g0) (assert (= 1 (.getTotalPacketCount g0))) (assert (= 0 (.getTotalPacketCount boat))) (synchronize g0 boat) (assert (= 0 (.getTotalPacketCount boat))) (assert (< (get lb0 0) (get (.getLowerBounds boat (array pair)) 0))) (synchronize boat g0) (synchronize g0 box))))'));

// Transfer a packet via the second gateway
eval(dd.parse('(dafn test2 (eps) (withEps eps (let (lb0 (first (.getLowerBounds boat pairs))) (.sendPacket box "boat119" 140 (new Buffer (array 3 4 5))) (synchronize box g1) (assert (= 1 (.getTotalPacketCount box))) (synchronize g1 boat)  (= (bigint.inc lb0) (first (.getLowerBounds boat pairs))))))'));

eval(dd.parse('(dafn runTest () (let (eps (makeEndPoints)) (withEps eps (.addPacketHandler boat handler) (test1 eps) (test2 eps) (assert (= (quote (139 140)) received)) (assert (= 1 (.getTotalPacketCount box))))))'));

describe('complex', function() {
  it('Should synchronize in a complex fashion', function(done) {
    runTest(done);
  });
});

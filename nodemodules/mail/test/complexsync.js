var mail2 = require('../mail2.sqlite.js');
var Q = require('q');
var assert = require('assert');
var synchronize = require('../sync2.js').synchronize;
var schema = require('../endpoint-schema.js');
var dd = require('dentdoche');

dd.setAsync(mail2.tryMakeAndResetEndPoint);
dd.setAsync(synchronize);
dd.setAsync(mail2.EndPoint.prototype.sendPacket);
dd.setAsync(mail2.EndPoint.prototype.getTotalPacketCount);

eval(dd.parse('(dafn makeEndPoints () (map (afn (fname name) ' +
              '(mail2.tryMakeAndResetEndPoint (+ "/tmp/testep_" fname ".db") name))' +
              '(quote ("a" "b" "c" "d")) ' +
              '(quote ("box119" "gateway" "gateway" "boat119"))))'));


eval(dd.parse('(dafn test1 (eps) (let ((box g0 g1 boat) eps) (.sendPacket box "boat119" 139 (new Buffer (array 0 1 2 3 4))) (synchronize box g0) (assert (= 1 (.getTotalPacketCount g0)))))'));

eval(dd.parse('(dafn runTest () (let (eps (makeEndPoints)) (test1 eps) (console.log "Done")))'));

describe('complex', function() {
  it('Should synchronize in a complex fashion', function(done) {
    runTest(done);
  });
});

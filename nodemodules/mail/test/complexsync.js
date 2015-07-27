var mail2 = require('../mail2.sqlite.js');
var Q = require('q');
var assert = require('assert');
var synchronize = require('../sync2.js').synchronize;
var schema = require('../endpoint-schema.js');

function makeEndPoints() {
  var names = ['box119', 'gateway', 'gateway', 'boat119'];
  return Q.all(names.map(function(name, index) {
    return Q.nfcall(
      mail2.tryMakeAndResetEndPoint, '/tmp/testbox' + index + '.db',
      name);
  }));
}

describe('complex', function() {
  it('Should synchronize in a complex fashion', function(done) {
    makeEndPoints().then(function(eps) {
      box = eps[0];
      gatewayA = eps[1];
      gatewayB = eps[2];
      boat = eps[3];
      assert.equal(box.name, 'box119');
      assert.equal(boat.name, 'boat119');
      schema.makeVerbose(boat);

      // Send the first file
      Q.ninvoke(box, 'sendPacket', 'boat119', 119, new Buffer([1, 2, 3]))
        .then(function() {
          return Q.nfcall(synchronize, box, gatewayA);
        }).then(function() {
          return Q.nfcall(synchronize, gatewayA, boat);
        }).then(function() {
          //return Q.ninvoke(boat, 'getPackets');
          return Q.ninvoke(boat, 'disp');
        }).then(function(packets) {
          console.log('The packets are');
          console.log(packets);
          done();
        });
        // }).then(function() {
        //   return Q.ninvoke(boat, 'disp');
        // }).then(function() {
        //   return Q.ninvoke(boat, 'hasPacket', function(packet) {
        //     return packet.dst == 'boat119' && packet.label == 119 && packet.src == 'box119';
        //   });
        // }).then(function(received) {
        //   console.log('Received???');
        //   assert(received);
        //   done();
        // }).catch(done);
      
    });
  });
});

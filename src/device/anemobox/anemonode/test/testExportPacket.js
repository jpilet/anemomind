var anemonode = require('../build/Release/anemonode');
var assert = require('assert');

var n2ksrc = new anemonode.Nmea2000Source();

function expectError(f) {
  var caught = false;
  try {
    f();
  } catch (e) {
    caught = true;
  }
  assert(caught);
}

describe('JsNmea2000Source::Export', function() {
  it('exportPackets', function() {
    var packets = n2ksrc.exportPackets(129029);

    // When running the unit tests, we are probably
    // not connected, so we will not get any packets.
    assert(packets.length == 0);

    // TODO: Test more things...
  });
  
  it('exportPackets, bad calls', function() {
    expectError(function() {
      n2ksrc.exportPackets();
    });

    expectError(function() {
      n2ksrc.exportPackets("kattskit");
    });
  });
});

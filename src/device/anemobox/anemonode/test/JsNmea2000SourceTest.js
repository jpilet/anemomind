var assert = require('assert');
var anemonode = require('../build/Release/anemonode');

var gnssData = require('./test_n2kgps.js').testData0;

var boxid = "kattskit";
var virtDevBits = 2;
var maxNumVirtualDevices = 1 << virtDevBits;
var baseSerialNumber =
      (parseInt(boxid, 16) & ((1 << 21 + 1 - virtDevBits) - 1)) << virtDevBits;



var nmea2000 = new anemonode.NMEA2000([
  {
    // product:
    serialCode: boxid + '-0',
    productCode: 140,
    model:"Anemobox logger",
    softwareVersion: '-1',
    modelVersion: 'Anemobox v1',
    loadEquivalency : 3,
    nmea2000Version: 2101,
    certificationLevel: 0xff,
    // device:
    uniqueNumber: baseSerialNumber + 0,
    manufacturerCode: 2040,
    deviceFunction: 140, // traffic logger
    deviceClass: 10,
    source: 42
  }]);

var src = new anemonode.Nmea2000Source(nmea2000);

assert(src);

function implies(a, b) {return !a || b;}
function isSubstringOf(a, b) {
  return b.indexOf(a) != -1;
}

function testSend(src, data, expectedError) {
  var result = null;
  try {
    src.send(data);
  } catch (e) {
    result = e.toString();
  }
  if (!(
    ((expectedError == null) == (result == null))
      && implies(expectedError != null, isSubstringOf(
        expectedError, result)))) {
    throw new Error("Expected error '" 
                    + expectedError + "' but got '" + result);
  }
}


describe('Try the send method', function() {
  it('Send GNSS', function() {
    gnssData.deviceIndex = 0;
    testSend(src, [
      gnssData
    ], "send");

    gnssData.longitude = null;
    testSend(src, [
      gnssData
    ], "send");

    gnssData.longitude = "kattskit";
    testSend(src, [
      gnssData
    ], "longitude");
  });

  it('Test zis send method', function() {
    testSend(src, [{}], "PGN missing");
    testSend(src, [{longitude: 9, latitude: 11}], "PGN missing");
    testSend(src, [{pgn: 119, deviceIndex: 0, latitude: 11}], "not supported");
    testSend(src, [{
      pgn: 129025, 
      latitude: 11, longitude: 13.0
    }], "deviceIndex");
    testSend(src, [{
      pgn: 129025, 
      deviceIndex: 0,
      latitude: 11, longitude: 13.0
    }], "send");
    testSend(src, [{
      pgn: 129025, 
      deviceIndex: 0,
      latitude: [11, "deg"], longitude: [13.0, "rad"]
    }], "send");
    testSend(src, [{
      pgn: 129025, 
      deviceIndex: 0,
      latitude: [11, "deg"], longitude: [13.0, "kattskit"]
      // Longitude provided, but on wrong format.
    }], "longitude");
    testSend(src, [{
      pgn: 129025, 
      deviceIndex: 0,
      latitude: [11, "deg"],
      // Missing longitude is not a problem? Or should we require it?
    }], "send");
  });
});

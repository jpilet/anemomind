var assert = require('assert');
var anemonode = require('../build/Release/anemonode');
try {
var can = require('socketcan');
} catch(e) { }

var testData = require('./n2kgps_data.js');
var packets = testData[0].packets;
var pgnTable = require('../components/pgntable.js');


function hasField(f) {return function(p) {return f in p;};}
var gnssData = packets.filter(hasField("longitude"))[0];
var cogSog = packets.filter(hasField("cog"))[0];

console.log("cogSog %j", cogSog);

var boxid = "kattskit";
var virtDevBits = 2;
var maxNumVirtualDevices = 1 << virtDevBits;
var baseSerialNumber =
      (parseInt(boxid, 16) & ((1 << 21 + 1 - virtDevBits) - 1)) << virtDevBits;


function canPacketReceived(msg) {
  if (nmea2000) {
    nmea2000.pushCanFrame(msg);
  }
}

var channel = null;
try {
  channel = can.createRawChannel("can0", true /* ask for timestamps */);
  channel.start();
  channel.addListener("onMessage", canPacketReceived);
} catch (e) {
  console.log("Failed to start NMEA2000");
  channel = null;
}

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

nmea2000.setSendCanFrame(function(id, data) {
  console.log('id=%s data=%s', id.toString(16), data.toString("hex"));

  if (channel) {
    var msg = { id: id, data: data, ext: true };
    var r = channel.send(msg);
    return r > 0;
  } else {
    return true;
  }
});
nmea2000.open();

function callParseMessages() {
  for (var i = 0; i < 5; i++) {
    nmea2000.parseMessages();
  }
}

callParseMessages();



assert(src);

function implies(a, b) {return !a || b;}
function isSubstringOf(a, b) {
  if (a == null || b == null) {
    return false;
  }
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

if (typeof describe != 'function') {
  // direct invocation without mocha
  describe = function(name, f) { f(function() { }); };
  it = function(name, f) { f(function() { }); }
}


describe('Try the send method', function() {
  it('Good cog and sog', function() {
    //"cog":[237.7,"deg"],"sog":[7.105,"knots"]

    var c = cogSog.cog;
    var s = cogSog.sog;
    assert(230 < c[0] && c[0] < 240);
    assert(6 < s[0] && s[0] < 8);
  });

  it('Send GNSS', function(done) {
    var intrvl = setInterval(callParseMessages, 100);
    setTimeout(function() {
      clearInterval(intrvl);

      gnssData.deviceIndex = 0;
      testSend(src, [
        gnssData
      ], null);
      callParseMessages();

      // Missing longitude should also be OK
      gnssData.longitude = null;
      testSend(src, [
        gnssData
      ], null);
      callParseMessages();

      // Deliberate corruption:
      gnssData.longitude = "kattskit";

      testSend(src, [
        gnssData
      ], "longitude");
      callParseMessages();
      
      // Put back a valid value
      gnssData.longitude = [3.44, "deg"];
      // ...and corrupt the geoidal separation
      gnssData.geoidalSeparation = "mmm";
      testSend(src, [
        gnssData
      ], "geoidal");
      callParseMessages();



      // Tests with PositionRapidUpdate
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
      }], null);


      testSend(src, [{
        pgn: 129025, 
        deviceIndex: 0,
        latitude: [11, "deg"], longitude: [13.0, "rad"]
      }], null);

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
      }], null);



      // Tests with WindData
      testSend(src, [{
        pgn: 130306,
        deviceIndex: 0,
        windSpeed: [8.0, "m/s"],
        windAngle: [120.4, "degrees"],
        reference: 0
      }], null);

      testSend(src, [{
        pgn: 130306,
        deviceIndex: 0,
        windSpeed: 1.4,
        windAngle: 120.4,
        reference: 0
      }], null);


      testSend(src, [{
        pgn: 130306,
        deviceIndex: 0,
        windSpeed: [8.0, "degrees"],
        windAngle: [120.4, "degrees"],
        reference: 0
      }], "windSpeed");


      //// Tests with TimeDate
      testSend(src, [{
        pgn: 129033,
        deviceIndex: 0,
        time: 7000,
        date: 7900,
        localOffset: 180
      }], null);
      
      testSend(src, [{
        pgn: 129033,
        deviceIndex: 0,
        time: "asdf",
        date: 7900,
        localOffset: 180
      }], "time");

      testSend(src, [{
        pgn: pgnTable.BandGVmgPerformance,
        deviceIndex: 0,
        vmgPerformance: 0.7 // 70%
      }], null);

      done();
    }, 1500);
  });
});


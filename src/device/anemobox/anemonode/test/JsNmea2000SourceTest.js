var assert = require('assert');
var anemonode = require('../build/Release/anemonode');

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

describe('Try the send method', function() {
  it('Should produce an error', function() {
    console.log('src is');
    console.log(src);
    for (var i in src) {
      console.log('Key ' + i);
    }
    src.send([{}]);
  });
});

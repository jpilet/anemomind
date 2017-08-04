var nf = require('../components/NmeaUtils.js');
var jsonData = require('./i2c_gps_data.json');
var buffers = jsonData.map(function(d) {return new Buffer(d);});
var assert = require('assert');

function addToArray(dst, x) {
  dst.push(x);
  return dst;
}

function parseNmeaChunks(testData) {
  return testData.reduce(
    nf.sample(addToArray), []);
}

describe('nmeaFilter', function() {
  it('parseNmea', function() {
    var sample = nf.sample(addToArray);

    var garbage = ["asdfasdf\nasdfasdf", "\n98980\nas", "dfasdfsad\nas", "dfasdf"];
    var testData = garbage.concat(buffers).concat(garbage);
    var output = parseNmeaChunks(testData);
    assert(!output.empty);
    var s = output[3] + '';
    assert(s.slice(1, 6) == "GNRMC");
    assert(s.slice(s.length-2, s.length) == "\r\n");
  });
});

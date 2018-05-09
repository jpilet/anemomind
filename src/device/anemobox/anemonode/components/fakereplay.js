var assert = require('assert');

var anemonode = require('../build/Release/anemonode');

var intervalSeconds = 0.5;
var source = "fakeReplay";

function time2angle(timeSeconds) { // degrees
  return timeSeconds*1.0 % 360;
}

function time2vel(timeSeconds) { // knots
  return timeSeconds*0.1 % 10.0;
}

function makeFakeData(timeSeconds) {
  var sampleAngle = time2angle(timeSeconds);
  var sampleVel = time2vel(timeSeconds);
  return {
    // Angles
    'awa': sampleAngle,
    'twa': sampleAngle,
    'twdir': sampleAngle,
    'gpsBearing': sampleAngle,
    'magHdg': sampleAngle,

    // Velocities
    'vmg': sampleVel,
    'targetVmg': sampleVel,
    'aws': sampleVel,
    'gpsSpeed': sampleVel,
    'aws': sampleVel,
    'watSpeed': sampleVel
  };
}

var counter = 0;
function start() {
  var dispatchers = anemonode.dispatcher.values;
  var intervalMillis = Math.round(intervalSeconds*1000.0);;
  setInterval(function() {
    var data = makeFakeData(counter*intervalSeconds);
    for (var key in data) {
      assert(key in dispatchers);
      dispatchers[key].setValue(source, data[key]);
    }
    counter++;
  }, intervalMillis);
}

module.exports.start = start;

var n2k = require('../components/nmea2000');
var anemonode = require('../build/Release/anemonode');

console.log("Start the nmea 2000 service");
n2k.startNmea2000({outputNmea2000: true});

var counter = 0;

var channels = ["tws", "twdir", "twa"];

/// Post things to the dispatcher that we expect will be sent to the CAN bus
var ivl = setInterval(function() {
  var value = counter*0.1;
  for (var i = 0; i < channels.length; i++) {
    var what = channels[i];
    anemonode.dispatcher.values[what].setValue(n2k.anemomindEstimatorSource, value);
    // No logging here, because it is important that the values come
    // close together in time, and logging would delay that.
  }
  console.log("setValue for %j to %j", channels, value);
  counter++;
}, 100);


/// Wait for some time
setTimeout(function() {
  console.log("We are done");
  clearInterval(ivl);
}, 30000);

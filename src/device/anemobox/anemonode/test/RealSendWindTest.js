var n2k = require('../components/nmea2000');
var anemonode = require('../build/Release/anemonode');

console.log("Start the nmea 2000 service");
n2k.startNmea2000();

console.log("Start the wind sending mechanism");
n2k.startSendingWindPackets();


var counter = 0;

var channels = ["twa", "tws", "twdir"];

/// Post things to the dispatcher that we expect will be sent to the CAN bus
var ivl = setInterval(function() {
	var what = channels[counter % channels.length];
	var value = counter*0.1;
	anemonode.dispatcher.values[what].setValue(n2k.anemomindEstimatorSource, value);
	console.log("setValue for %s to %j", what, value);
	counter++;
}, 100);


/// Wait for some time
setTimeout(function() {
	console.log("We are done");
	clearInterval(ivl);
}, 30000);

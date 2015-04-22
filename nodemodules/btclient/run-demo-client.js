var assert = require('assert');
var client = require('./btclient.js');

// Register the functions that we would like to call.
client.setCalls(['testAdd']);


// A function that make a remote call in order to add two numbers.
function makeRandomCall(cb) {
    var a = 119*Math.random();
    var b = 119*Math.random();
    client.testAdd(a, b, function(err, sum) {
	if (err) {
	    cb(err);
	} else {
	    assert(a + b == sum);
	    console.log('' + a + ' + ' + b + ' = ' + sum);
	    cb();
	}
    });
}

// Make two different calls.
makeRandomCall(
    function(err) {
	console.log('Evaluate a second call.');
	makeRandomCall(function() {});
    }
);



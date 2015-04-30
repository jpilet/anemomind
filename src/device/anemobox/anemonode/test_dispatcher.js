var fs = require('fs');
var anemonode = require('./build/Release/anemonode');

// Inspect the anemonode object
console.warn(anemonode);

// Try subscribe/unsubscribe functions
var numCalls = 0;
var subsIndex = anemonode.dispatcher.awa.subscribe(function(val) {
  var description = anemonode.dispatcher.awa.description;
  console.log(description + ' changed to: ' + val);
  if (++numCalls == 3) {
    // stop notifying.
    console.log('Unsubscribing from ' + description);
    anemonode.dispatcher.awa.unsubscribe(subsIndex);
  }
});

anemonode.dispatcher.awa.setValue("js test", 1);
console.log('awa ' + anemonode.dispatcher.awa.value());

var nmeaSource = new anemonode.Nmea0183Source();
console.warn(nmeaSource.process);

function format(x) {
  return JSON.stringify(x);
}

function printHistory(field) {
  for (var i = 0; i < anemonode.dispatcher[field].length(); ++i) {
    var dispatchData = anemonode.dispatcher[field];
    console.log(anemonode.dispatcher[field].description
                + ': ' + format(dispatchData.value(i))
                + ' ' + anemonode.dispatcher[field].unit
                + ' set on: ' + dispatchData.time(i).toLocaleString());
  }
}
// Read some NMEA data.
fs.readFile("../../../../datasets/tinylog.txt", function (err, data ) {
  if (err) {
    console.warn(err);
  } else {
    nmeaSource.process(data);
  }

  // List all available values
  for (var i in anemonode.dispatcher) {
    console.log(anemonode.dispatcher[i].description
                + ": " + anemonode.dispatcher[i].value());
  }

  printHistory('awa');
  printHistory('twa');
  printHistory('pos');
  printHistory('dateTime');

});
